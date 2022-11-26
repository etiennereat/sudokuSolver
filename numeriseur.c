#include "grille.h"
#include "asprise_ocr_api.h"

               
#define PBSTR "||||||||||||||||||||SCAN||IN||PROGRESS||||||||||||||||||||||"
#define PBWIDTH 60

void printProgress(int percentage) {
    int val = (int) percentage;
    int lpad = (int) (percentage * PBWIDTH)/100;
    int rpad = PBWIDTH - lpad;
    printf("\r%3d%% [%.*s%*s]", val, lpad, PBSTR, rpad, "");
    fflush(stdout);
}

static void
fatal_error (const char * message, ...)
{
    va_list args;
    va_start (args, message);
    vfprintf (stderr, message, args);
    va_end (args);
    exit (EXIT_FAILURE);
}


// function to swap elements
void swap(int *a, int *b) {
  int t = *a;
  *a = *b;
  *b = t;
}

// function to find the partition position
int partition(int array[], int low, int high) {
  
  // select the rightmost element as pivot
  int pivot = array[high];
  
  // pointer for greater element
  int i = (low - 1);

  // traverse each element of the array
  // compare them with the pivot
  for (int j = low; j < high; j++) {
    if (array[j] <= pivot) {
        
      // if element smaller than pivot is found
      // swap it with the greater element pointed by i
      i++;
      
      // swap element at i with element at j
      swap(&array[i], &array[j]);
    }
  }

  // swap the pivot element with the greater element at i
  swap(&array[i + 1], &array[high]);
  
  // return the partition point
  return (i + 1);
}

void quickSort(int array[], int low, int high) {
  if (low < high) {
    
    // find the pivot element such that
    // elements smaller than pivot are on left of pivot
    // elements greater than pivot are on right of pivot
    int pi = partition(array, low, high);
    
    // recursive call on the left of pivot
    quickSort(array, low, pi - 1);
    
    // recursive call on the right of pivot
    quickSort(array, pi + 1, high);
  }
}

void printArray(int array[], int size) {
  for (int i = 0; i < size; ++i) {
    printf("%d  ", array[i]);
  }
  printf("\n");
}


int storeLineScore(int score,int nbLine,int storeLines[100][2]){
    for(int i = 0 ; i < 100 ; i++ ){
        if(score > storeLines[i][0]){    
            for(int j = 99 ; j > i ; j--){
                storeLines[j][0] = storeLines[j-1][0];
                storeLines[j][1] = storeLines[j-1][1];
            }
            storeLines[i][0] = score;
            storeLines[i][1] = nbLine;
            return 1;
        }
    }
    return 0;
}   


void detect_lines(png_bytepp rows,int rowbytes, int height, int width, int result[10]){
    int storeLines[100][2];
    for(int i = 0 ; i<100;i++){
        storeLines[i][0] = 0;
        storeLines[i][1] = 0;
    }
    //search lines
    int tolerence = 0;
    int delay = 0;
    for (int j = 0; j < height; j++) {
        png_bytep row;
        row = rows[j];
        int compteur = 0;
        int score = 0;
        for (int i = 0; i < rowbytes; i++) {
            png_byte pixel;
            pixel = row[i];
            if (pixel < 64) {
                compteur++;
            }

        }
        storeLineScore(compteur,j,storeLines);
    }

//search max to detect ligne 
    int max=0;
    for(int i =0; i<100;i++){
        if(max < storeLines[i][0]){
            max = storeLines[i][0];
        }
    }

    int sortedLines[100];
    int nb = 0;
    for(int i =0; i<100;i++){
        if(max - max/9 < storeLines[i][0]){
            sortedLines[nb]=storeLines[i][1];
            nb++;
        }
    }
    quickSort(sortedLines,0,nb-1);

    int nb_index = 0;

    for(int i = 0; i< nb; i++){
        if(i == 0 || (sortedLines[i-1] + height/20 < ( sortedLines[i] ) )){
            result[nb_index] = sortedLines[i];
            nb_index++;
        }
    }



}

void detect_columnes(png_bytepp rows,int rowbytes, int height, int width, int result[10]){

    int storecolumnes[100][2];
    for(int i = 0 ; i<100;i++){
        storecolumnes[i][0] = 0;
        storecolumnes[i][1] = 0;
    }
    for (int i = 0; i < rowbytes; i++) {
        png_bytep row;
        int compteur = 0;
        int score = 0;
        for (int j = 0; j < height; j++) {
            row = rows[j];
            png_byte pixel;
            pixel = row[i];
            if (pixel < 64) {
                compteur++;
            }

        }
        storeLineScore(compteur,i,storecolumnes);
        
    }

    //search max to detect ligne 
    int max=0;
    for(int i =0; i<100;i++){
        if(max < storecolumnes[i][0]){
            max = storecolumnes[i][0];
        }
    }

    int sortedColomnes[100];
    int nb = 0;
    for(int i =0; i<100;i++){
        if(max - max/ 9 < storecolumnes[i][0]){
            sortedColomnes[nb]=storecolumnes[i][1];
            nb++;
        }
    }


    quickSort(sortedColomnes,0,nb-1);

    int nb_index = 0;

    for(int i = 0; i< nb; i++){
        if(i == 0 || (sortedColomnes[i-1] + rowbytes/10 < ( sortedColomnes[i]) )){
            result[nb_index] = sortedColomnes[i];
            nb_index++;
        }
    }


}

int detecte_value_of_cases(const char * fileImg, int storeLines[10], int storeColumnes[10], grille * grilleREF, int width, int height, int rowbytes){
    // CHANGE TO THE ACTUAL PATH to the folder where the aocr.dll or aocr.so locates in
   const char * libFolder = "/home/etienne/travail/projetPerso/sudokuSolver/";

   LIBRARY_HANDLE libHandle = dynamic_load_aocr_library(libFolder);

   // one time setup
   int setup = c_com_asprise_ocr_setup(0);
   if (setup != 1) {
      return 1;
   }

   // starts the ocr engine; the pointer must be of long long type
   long long ptrToApi = c_com_asprise_ocr_start("eng",  OCR_SPEED_FAST, NULL, NULL, NULL);
   if (ptrToApi == 0) {
      return 1;
   }
   for (int j = 0 ; j < 9; j++) {
        for (int i = 0 ; i < 9 ; i++){
            freopen("log.txt","w",stdout);
            char * s = c_com_asprise_ocr_recognize(ptrToApi, fileImg, -1, (storeColumnes[j]* width)/rowbytes,storeLines[i], width/8,height/8,
                OCR_RECOGNIZE_TYPE_TEXT,OCR_OUTPUT_FORMAT_PLAINTEXT,
                NULL,NULL,NULL);
            freopen("/dev/tty","w",stdout);
            printProgress((j*9+i)*100 / 80);
//            printf("\n\nscan at %d %d with %d %d : %s \n",(storeColumnes[j]* width)/rowbytes,storeLines[i], (storeColumnes[j+1]* width)/rowbytes - (storeColumnes[j]* width)/rowbytes, storeLines[i+1] - storeLines[i],s);
            switch(s[0]){
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                    set_in_grille(grilleREF,i,j,atoi(s));
                    break;
                case '*':
                    set_in_grille(grilleREF,i,j,9);
                    break;
                default :
                    set_in_grille(grilleREF,i,j,0);
            }
        }
   }
   c_com_asprise_ocr_stop(ptrToApi);
   return 0;
}

grille * read_png_to_grille (const char * png_file)
{
    //png variables managers
    png_structp	png_ptr;
    png_infop info_ptr;
    FILE * fp;
    png_uint_32 width;
    png_uint_32 height;
    int bit_depth;
    int color_type;
    int interlace_method;
    int compression_method;
    int filter_method;
    png_bytepp rows;
    int rowbytes;

    //analyse variables
    int storeLines[10];
    int storecolumnes[10];
    grille* grilleREF = init_grille();

    //try to open png 
    fp = fopen (png_file, "rb");
    if (! fp) {
	    fatal_error ("Cannot open '%s': %s\n", png_file, strerror (errno));
    }
    png_ptr = png_create_read_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (! png_ptr) {
	    fatal_error ("Cannot create PNG read structure");
    }
    info_ptr = png_create_info_struct (png_ptr);
    if (! png_ptr) {
	    fatal_error ("Cannot create PNG info structure");
    }

    //hook data from png 
    png_init_io (png_ptr, fp);
    png_read_png (png_ptr, info_ptr, 0, 0);
    png_get_IHDR (png_ptr, info_ptr, & width, & height, & bit_depth,
		  & color_type, & interlace_method, & compression_method,
		  & filter_method);
    rows = png_get_rows (png_ptr, info_ptr);
    rowbytes = png_get_rowbytes (png_ptr, info_ptr);

    //display data
    printf ("Width is %d, height is %d\n", width, height);
    printf ("Row bytes = %d\n", rowbytes);

    //search lines
    detect_lines(rows,rowbytes,height,width,storeLines);

    //search columnes
    detect_columnes(rows,rowbytes,height,width,storecolumnes);

    //detecte value in setuped case 
    detecte_value_of_cases(png_file,storeLines,storecolumnes,grilleREF,width,height,rowbytes);
    
    //close png 
    fclose(fp);

    return grilleREF;
}