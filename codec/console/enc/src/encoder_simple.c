#include "codec_def.h"
#include "codec_api.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char** argv)
{
#if defined(_DEBUG) && defined(_MSC_VER) && defined(WIN32)
  int Flag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
  Flag |= _CRTDBG_LEAK_CHECK_DF;
  _CrtSetDbgFlag(Flag);
  //_CrtSetBreakAlloc(24000);
#endif
#if !defined(RTOS)
  FILE* out_file_ = fopen("out.h264", "wb");
#endif
  ISVCEncoder* encoder_ = 0;
  int width = 176, height = 144;
  int rv = WelsCreateSVCEncoder(&encoder_);

  SEncParamExt param;
  memset(&param, 0, sizeof(SEncParamExt));
  param.iUsageType = CAMERA_VIDEO_REAL_TIME;
  param.fMaxFrameRate = 5;
  param.iPicWidth = width;
  param.iPicHeight = height;
  ///< target bitrate desired, in unit of bps
  param.iTargetBitrate = 40 * 1000;
  ///< the maximum bitrate, in unit of bps, set it to UNSPECIFIED_BIT_RATE if not needed
  param.iMaxBitrate = 60 * 1000;
  ///< rate control mode
  param.iRCMode = RC_BITRATE_MODE;
  ///< period of Intra frame, IDR frame period 400 frame
  param.uiIntraPeriod = 400;
  ///< 1 # 0: auto(dynamic imp. internal encoder); 1: multiple threads imp. disabled; lager than 1: count number of threads;
  param.iMultipleThreadIdc = 1;
  ///< False: don't skip frame even if VBV buffer overflow.True: allow skipping frames to keep the bitrate within limits
  param.bEnableFrameSkip = 1;
  ///< number of reference frame used
  param.iNumRefFrame = 1;
  ///< the maximum QP encoder supports
  param.iMaxQp = 49;
  ///< the minmum QP encoder supports
  param.iMinQp = 20;
  // The base spatial layer 0 is the only one we use.
  param.sSpatialLayers[0].iVideoWidth = param.iPicWidth;
  param.sSpatialLayers[0].iVideoHeight = param.iPicHeight;
  param.sSpatialLayers[0].fFrameRate = param.fMaxFrameRate;
  param.sSpatialLayers[0].iSpatialBitrate =
    param.iTargetBitrate;
  param.sSpatialLayers[0].iMaxSpatialBitrate =
    param.iMaxBitrate;
  // Slice num according to number of threads.
  param.sSpatialLayers[0].sSliceArgument.uiSliceMode = SM_SINGLE_SLICE;
  param.sSpatialLayers[0].uiProfileIdc = PRO_BASELINE;
  param.sSpatialLayers[0].uiLevelIdc = LEVEL_1_3;
  int ret = (*encoder_)->InitializeExt(encoder_, &param);

  int videoFormat = videoFormatI420;
  ret = (*encoder_)->SetOption(encoder_, ENCODER_OPTION_DATAFORMAT, &videoFormat);

  int frameSize = width * height * 3 / 2;
  unsigned char* buf = (unsigned char*)malloc(frameSize);
  int total_num = 10000;
  SFrameBSInfo info;
  memset(&info, 0, sizeof(SFrameBSInfo));
  SSourcePicture pic;
  memset(&pic, 0, sizeof(SSourcePicture));
  pic.iPicWidth = width;
  pic.iPicHeight = height;
  pic.iColorFormat = videoFormatI420;
  pic.iStride[0] = pic.iPicWidth;
  pic.iStride[1] = pic.iStride[2] = pic.iPicWidth >> 1;
  pic.pData[0] = buf;
  pic.pData[1] = buf + width * height;
  pic.pData[2] = pic.pData[1] + (width * height >> 2);
  for (int num = 0; num<total_num; num++) {
    //prepare input data
    rv = (*encoder_)->EncodeFrame(encoder_, &pic, &info);
    if (rv == cmResultSuccess) {
      if (info.eFrameType != videoFrameTypeSkip) {
        //output bitstream
        int iLayer = 0;
      int iFrameSize = 0;
      while (iLayer < info.iLayerNum) {
        SLayerBSInfo* pLayerBsInfo = &info.sLayerInfo[iLayer];
        if (pLayerBsInfo != NULL) {
          int iLayerSize = 0;
          int iNalIdx = pLayerBsInfo->iNalCount - 1;
          do {
            iLayerSize += pLayerBsInfo->pNalLengthInByte[iNalIdx];
            -- iNalIdx;
          } while (iNalIdx >= 0);
          if (pLayerBsInfo->uiSpatialId == 0) {
            unsigned char five_bits = pLayerBsInfo->pBsBuf[4] & 0x1f;
#if !defined(RTOS)
            if ((five_bits == 0x07) || (five_bits == 0x08)) { //sps or pps
              fwrite (pLayerBsInfo->pBsBuf, 1, iLayerSize, out_file_);
            } else {
              fwrite(pLayerBsInfo->pBsBuf, 1, iLayerSize, out_file_);
            }
          } else {
            fwrite(pLayerBsInfo->pBsBuf, 1, iLayerSize, out_file_);
          }
#endif
          iFrameSize += iLayerSize;
        }
        ++ iLayer;
      }
      }
    }
  }

  free(buf);
#if !defined(RTOS)
  fclose(out_file_);
#endif
  if (encoder_) {
    (*encoder_)->Uninitialize(encoder_);
    WelsDestroySVCEncoder(encoder_);
  }
}
