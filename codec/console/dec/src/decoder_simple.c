

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "codec_def.h"
#include "codec_api.h"

void Decoder(ISVCDecoder* pDecoder) {
  unsigned char * pBuf = NULL;
  unsigned char uiStartCode[4] = {0, 0, 0, 1};
  unsigned char* pData[3] = {NULL};
  unsigned char* pDst[3] = {NULL};
  SBufferInfo sDstBufInfo;
  int iSliceSize = -1;
  int iSliceIndex = 0;
  int iBufPos = 0;
  int i = 0;
  int iFrameCount = 0;
  int iEndOfStreamFlag = 0;
  do {
    FILE* pH264File = fopen ("d:\\rcrtc\\media\\I420_176_144.h264", "rb");
    if (pH264File == NULL) {
      break;
    }
    fseek (pH264File, 0L, SEEK_END);
    int iFileSize = (int) ftell (pH264File);
    fseek (pH264File, 0L, SEEK_SET);
    unsigned char* pBuf = malloc(iFileSize + 4);
    if (fread (pBuf, 1, iFileSize, pH264File) != (unsigned int)iFileSize) {
      fprintf (stderr, "Unable to read whole file\n");
      break;
    }

    memcpy (pBuf + iFileSize, &uiStartCode[0], 4); //confirmed_safe_unsafe_usage
    //TODO(hhool):
    //start_time;
    while(true) {
      if (iBufPos >= iFileSize) {
        iEndOfStreamFlag = true;
        if (iEndOfStreamFlag)
          (*pDecoder)->SetOption(pDecoder, DECODER_OPTION_END_OF_STREAM, (void*)&iEndOfStreamFlag);
        break;
      }
      for (i = 0; i < iFileSize; i++) {
        if ((pBuf[iBufPos + i] == 0 && pBuf[iBufPos + i + 1] == 0 && pBuf[iBufPos + i + 2] == 0 && pBuf[iBufPos + i + 3] == 1
             && i > 0) || (pBuf[iBufPos + i] == 0 && pBuf[iBufPos + i + 1] == 0 && pBuf[iBufPos + i + 2] == 1 && i > 0)) {
          break;
        }
      }
      
      iSliceSize = i;
      if (iSliceSize < 4) { //too small size, no effective data, ignore
        iBufPos += iSliceSize;
        continue;
      }

      pData[0] = NULL;
      pData[1] = NULL;
      pData[2] = NULL;
      memset (&sDstBufInfo, 0, sizeof (SBufferInfo));

      (*pDecoder)->DecodeFrameNoDelay(pDecoder, pBuf + iBufPos, iSliceSize, pData, &sDstBufInfo);
    
      if (sDstBufInfo.iBufferStatus == 1) {
        pDst[0] = pData[0];
        pDst[1] = pData[1];
        pDst[2] = pData[2];

        ++iFrameCount;
      }
      iBufPos += iSliceSize;
      ++iSliceIndex;
    }
  } while(false);
  //TODO(hhool):
  //stop_time;
}

int main(int iArgC, char* pArgV[]) {
  ISVCDecoder* pDecoder = NULL;
  if (WelsCreateDecoder (&pDecoder)  || (NULL == pDecoder)) {
    printf ("Create Decoder failed.\n");
    return 1;
  }
  SDecodingParam sDecParam = {0};
  sDecParam.sVideoProperty.size = sizeof (sDecParam.sVideoProperty);
  sDecParam.uiTargetDqLayer = (unsigned char) - 1;
  sDecParam.eEcActiveIdc = ERROR_CON_SLICE_COPY;
  sDecParam.sVideoProperty.eVideoBsType = VIDEO_BITSTREAM_DEFAULT;

  if ((*pDecoder)->Initialize (pDecoder, &sDecParam)) {
    printf ("Decoder initialization failed.\n");
    return 1;
  }

  Decoder(pDecoder);

  if (pDecoder) {
    (*pDecoder)->Uninitialize(pDecoder);

    WelsDestroyDecoder (pDecoder);
  }
  return 0;
}