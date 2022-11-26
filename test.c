#include "asprise_ocr_api.h"


void testOcr(char * filename) {
   // CHANGE TO THE ACTUAL PATH to the folder where the aocr.dll or aocr.so locates in
   const char * libFolder = "/home/etienne/travail/projetPerso/sudokuSolver/";

   // CHANGE TO THE ACTUAL PATH to the input image, can be jpeg, gif, png, tiff or pdf.
   const char * fileImg = filename;

   LIBRARY_HANDLE libHandle = dynamic_load_aocr_library(libFolder);

   // one time setup
   int setup = c_com_asprise_ocr_setup(0);
   if (setup != 1) {
      return;
   }

   // starts the ocr engine; the pointer must be of long long type
   long long ptrToApi = c_com_asprise_ocr_start("eng",  OCR_SPEED_FAST,
     "START_PROP_DICT_CUSTOM_TEMPLATES_FILE=templates.txt", "|", "=");
   if (ptrToApi == 0) {
      return;
   }

   char * s = c_com_asprise_ocr_recognize(ptrToApi, fileImg, -1,67, 5, 72, 73,
      OCR_RECOGNIZE_TYPE_TEXT, OCR_OUTPUT_FORMAT_PDF,"PROP_PDF_OUTPUT_FILE=result.pdf|PROP_PDF_OUTPUT_TEXT_VISIBLE=true|\
      PROP_PDF_OUTPUT_RETURN_TEXT=text", "|", "=");
    printf("%s\n",s);

   // do more recognition here ...

   // finally, stops the OCR engine.
   c_com_asprise_ocr_stop(ptrToApi);

}

int main(int argc,char ** argv) { // Entry point
   testOcr(argv[1]);
   return 0;
}