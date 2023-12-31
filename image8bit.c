
/// image8bit - A simple image processing module.
///
/// This module is part of a programming project
/// for the course AED, DETI / UA.PT
///
/// You may freely use and modify this code, at your own risk,
/// as long as you give proper credit to the original and subsequent authors.
///
/// João Manuel Rodrigues <jmr@ua.pt>
/// 2013, 2023

// Student authors (fill in below):
// NMec: 112549
//       114288
//
// Name:José Carlos Wanderley Galdino Neto
//      Miguel Rosa Vicente
//

#include "image8bit.h"

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <math.h>
#include "instrumentation.h"

// Clients should use images only through variables of type Image,
// which are pointers to the image structure, and should not access the
// structure fields directly.

const uint8 PixMax = 255;

struct image
{
  int width;
  int height;
  int maxval;   // maximum gray value (pixels with maxval are pure WHITE)
  uint8 *pixel; // pixel data (a raster scan)
};

// Additional information:  man 3 errno;  man 3 error;

// Variable to preserve errno temporarily
static int errsave = 0;

// Error cause
static char *errCause;

char *ImageErrMsg()
{ ///
  return errCause;
}

static int check(int condition, const char *failmsg)
{
  errCause = (char *)(condition ? "" : failmsg);
  return condition;
}

void ImageInit(void)
{ ///
  InstrCalibrate();

  InstrName[0] = "pixmem"; // InstrCount[0] will count pixel array acesses
  InstrName[1] = "LocCount"; // InstrCount[1] will count LocateSubimages  

}

#define PIXMEM InstrCount[0]
#define LocateComparisons InstrCount[1]


/// Image management functions

/// Create a new black image.
///   width, height : the dimensions of the new image.
///   maxval: the maximum gray level (corresponding to white).
/// Requires: width and height must be non-negative, maxval > 0.
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
/// On failure, returns NULL and errno/errCause are set accordingly.
Image ImageCreate(int width, int height, uint8 maxval)
{ ///
  assert(width >= 0);
  assert(height >= 0);
  assert(0 < maxval && maxval <= PixMax);

  Image newImg = (Image)malloc(sizeof(struct image)); // Aloca a memória para a imagem

  if (!check(newImg != NULL, "Failed to allocate memory for image")) 
  {
    return NULL; //Caso nao consiga, retorna NULL
  }

  // Atribui os parametros para a imagem
  newImg->width = width;
  newImg->height = height;
  newImg->maxval = maxval;
  
  // Aloca a memória para o array de pixels da imagem
  newImg->pixel = (uint8 *)calloc(width * height, sizeof(uint8));

  if (!check(newImg->pixel != NULL, "Failed to allocate memory for pixels"))
  {
    free(newImg); //Caso nao consiga, retorna NULL e libera o espaço da imagem
    return NULL;
  }

  return newImg;
}

/// Destroy the image pointed to by (*imgp).
///   imgp : address of an Image variable.
/// If (*imgp)==NULL, no operation is performed.
/// Ensures: (*imgp)==NULL.
/// Should never fail, and should preserve global errno/errCause.
void ImageDestroy(Image *imgp)
{ ///
  assert(imgp != NULL);
  //Libera o espaço da imagem e do array de pixels
  free((*imgp)->pixel);
  free(*imgp);
  imgp = NULL;
}

/// PGM file operations

// Match and skip 0 or more comment lines in file f.
// Comments start with a # and continue until the end-of-line, inclusive.
// Returns the number of comments skipped.
static int skipComments(FILE *f)
{
  char c;
  int i = 0;
  while (fscanf(f, "#%*[^\n]%c", &c) == 1 && c == '\n')
 
  {
    i++;
  }
  return i;
}

/// Load a raw PGM file.
/// Only 8 bit PGM files are accepted.
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
/// On failure, returns NULL and errno/errCause are set accordingly.
Image ImageLoad(const char *filename)
{ ///
  int w, h;
  int maxval;
  char c;
  FILE *f = NULL;
  Image img = NULL;

  int success =
      check((f = fopen(filename, "rb")) != NULL, "Open failed") &&
      // Parse PGM header
      check(fscanf(f, "P%c ", &c) == 1 && c == '5', "Invalid file format") &&
      skipComments(f) >= 0 &&
      check(fscanf(f, "%d ", &w) == 1 && w >= 0, "Invalid width") &&
      skipComments(f) >= 0 &&
      check(fscanf(f, "%d ", &h) == 1 && h >= 0, "Invalid height") &&
      skipComments(f) >= 0 &&
      check(fscanf(f, "%d", &maxval) == 1 && 0 < maxval && maxval <= (int)PixMax, "Invalid maxval") &&
      check(fscanf(f, "%c", &c) == 1 && isspace(c), "Whitespace expected") &&
      // Allocate image
      (img = ImageCreate(w, h, (uint8)maxval)) != NULL &&
      // Read pixels
      check(fread(img->pixel, sizeof(uint8), w * h, f) == w * h, "Reading pixels");
  PIXMEM += (unsigned long)(w * h); // count pixel memory accesses
  // Cleanup
  if (!success)
 
  {
    errsave = errno;
    ImageDestroy(&img);
    errno = errsave;
  }
  if (f != NULL)
   
    fclose(f);
  return img;
}

/// Save image to PGM file.
/// On success, returns nonzero.
/// On failure, returns 0, errno/errCause are set appropriately, and
/// a partial and invalid file may be left in the system.
int ImageSave(Image img, const char *filename)
{ ///
  assert(img != NULL);
  int w = img->width;
  int h = img->height;
  uint8 maxval = img->maxval;
  FILE *f = NULL;

  int success =
      check((f = fopen(filename, "wb")) != NULL, "Open failed") &&
      check(fprintf(f, "P5\n%d %d\n%u\n", w, h, maxval) > 0, "Writing header failed") &&
      check(fwrite(img->pixel, sizeof(uint8), w * h, f) == w * h, "Writing pixels failed");
  PIXMEM += (unsigned long)(w * h); // count pixel memory accesses
  // Cleanup
  if (f != NULL)
   
    fclose(f);
  return success;
}

/// Information queries
/// These functions do not modify the image and never fail.

/// Get image width
int ImageWidth(Image img)
{ ///
  assert(img != NULL);
  return img->width; //Retorna largura
}

/// Get image width
int ImageHeight(Image img)
{ ///
  assert(img != NULL);
  return img->height; //Retorna altura
}

/// Get image maximum gray level
int ImageMaxval(Image img)
{ ///
  assert(img != NULL);
  return img->maxval; //Retorna maxval
}

/// Pixel stats
/// Find the minimum and maximum gray levels in image.
/// On return,
/// *min is set to the minimum gray level in the image,
/// *max is set to the maximum.
void ImageStats(Image img, uint8 *min, uint8 *max)
{ ///
  assert(img != NULL);

  *min = ImageGetPixel(img, 0, 0);
  *max = ImageGetPixel(img, 0, 0);

  //Percorre todos os pixels e compara eles com *min e *max, caso tenham um valor de cinza maior
  //substituem o valor de max, caso seja menor, substituem o valor de min 
  for (size_t i = 1; i < img->width*img->height; i++) 
  {
    if (img->pixel[i] > *max)
    {
      *max = img->pixel[i];
    }
    else if (img->pixel[i] < *min)
    {
      *min = img->pixel[i];
    }

    if (*min == 0 && *max == img->maxval)
    {
      return;
    }
  }

}


/// Check if pixel (x,y) is inside img.
int ImageValidPos(Image img, int x, int y)
{ ///
  assert(img != NULL);
  return (0 <= x && x < img->width) && (0 <= y && y < img->height);
}


/// Check if rectangular area (x,y,w,h) is completely inside img.
int ImageValidRect(Image img, int x, int y, int w, int h)
{ 
  assert(img != NULL);
  return(0 <= x && x + w <= img->width) && (0 <= y && y + h <= img->height);

}

/// Pixel get & set operations

/// These are the primitive operations to access and modify a single pixel
/// in the image.
/// These are very simple, but fundamental operations, which may be used to
/// These are very simple, but fundamental operations, which may be used to
/// implement more complex operations.

// Transform (x, y) coords into linear pixel index.
// This internal function is used in ImageGetPixel / ImageSetPixel.
// This internal function is used in ImageGetPixel / ImageSetPixel.
// The returned index must satisfy (0 <= index < img->width*img->height)
static inline int G(Image img, int x, int y)
{
  int index;
  int width = ImageWidth(img);
  index = width*(y)+x; //calcula posição do pixel, considerando (0,0) como ponto inicial 
  assert(0 <= index && index < width * ImageHeight(img));
  return index;
}

/// Get the pixel's at position (x,y) level.
uint8 ImageGetPixel(Image img, int x, int y)
{ ///
  assert(img != NULL);
  assert(ImageValidPos(img, x, y));
  PIXMEM += 1; // count one pixel access (read)
  return img->pixel[G(img, x, y)];
}


/// Set the pixel at position (x,y) to new level.
void ImageSetPixel(Image img, int x, int y, uint8 level)
{ ///
  assert(img != NULL);
  assert(ImageValidPos(img, x, y));
  PIXMEM += 1; // count one pixel access (store)
  img->pixel[G(img, x, y)] = level;
}


/// Pixel transformations

/// These functions modify the pixel levels in an image, but do not change
/// pixel positions or image geometry in any way.
/// All of these functions modify the image in-place: no allocation involved.
/// They never fail.

/// Transform image to negative image.
/// This transforms dark pixels to light pixels and vice-versa,
/// resulting in a "photographic negative" effect.
void ImageNegative(Image img)
{ ///
  assert(img != NULL);
  int maxval = ImageMaxval(img);
  for (size_t i = 0; i < ImageWidth(img)*ImageHeight(img); i++)
  {
    //Para todos os pixels subtrai maxval do valor de cinza, conseguindo o negativo
    img->pixel[i] = maxval - img->pixel[i]; 
  }
  
}

/// Apply threshold to image.
/// Transform all pixels with level<thr to black (0) and
/// all pixels with level>=thr to white (maxval).

void ImageThreshold(Image img, uint8 thr)
{ ///
  assert(img != NULL);
  int maxval = ImageMaxval(img);

  //Loop for que percorre todos os pixels, e caso seu valor de cinza seja menor que a thr, transforma em branco,
  //caso seja maior, transforma em preto
  for (size_t i = 0; i < img->width*img->height; i++)
  {
    if (img->pixel[i] < thr)
    {
      img->pixel[i] = 0;
    }
    else
    {
      img->pixel[i] = maxval;
    }
  }
}

/// Brighten image by a factor.
/// Multiply each pixel level by a factor, but saturate at maxval.
/// This will brighten the image if factor>1.0 and
/// darken the image if factor<1.0.
void ImageBrighten(Image img, double factor)
{ ///
  assert(img != NULL);
  assert (factor >= 0.0);
  int maxval = ImageMaxval(img);
  
  //Percorre todos os pixels
  for (size_t i = 0; i < ImageWidth(img)*ImageHeight(img); i++)
  {
    //Multiplica o valor do pixel pelo fator
    double aux = img->pixel[i] * factor;

    if (aux > maxval)
    {
      //Certifica-se que o valor não ultrapassa maxval
      img->pixel[i] = maxval;
    }
    else
    { 
      //Arredonda o aux e coloca no lugar do pixel
      img->pixel[i] = (uint8)round(aux);
    }
  }
}

/// Geometric transformations

/// These functions apply geometric transformations to an image,
/// returning a new image as a result.
///
///
/// Success and failure are treated as in ImageCreate:
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
/// On failure, returns NULL and errno/errCause are set accordingly.
///
///
/// Rotate an image.
/// Returns a rotated version of the image.
/// The rotation is 90 degrees anti-clockwise.
/// Ensures: The original img is not modified.
///
///
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
/// On failure, returns NULL and errno/errCause are set accordingly.
Image ImageRotate(Image img)
{ ///
  assert(img != NULL);

  int width = ImageWidth(img);
  int height = ImageHeight(img);
  int maxval = ImageMaxval(img);

  Image newImage = ImageCreate(height, width, maxval);
  if (newImage == NULL)
  {
    return NULL;
  }
  //Percorre todos os pixels da imagem e faz a transformação que permite rotacionar os pixeis
  //atribuindo os pixels à nova imagem criada
  for (size_t x = 0; x < width; x++)
  {
    for (size_t y = 0; y < height; y++)
    {
      ImageSetPixel(newImage, y, width - x - 1, ImageGetPixel(img, x, y));
    }
  }	

  return newImage;
}

/// Mirror an image = flip left-right.
/// Returns a mirrored version of the image.
/// Ensures: The original img is not modified.
///
///
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
/// On failure, returns NULL and errno/errCause are set accordingly.

Image ImageMirror(Image img)
{ ///
  assert(img != NULL);
  
  Image newImage = ImageCreate(img->width, img->height, img->maxval);
  if (newImage == NULL)
  {
    return NULL;
  }

  //Percorre todos os pixels da imagem e faz a transformação que permite esplehar os pixeis
  //atribuindo os pixels à nova imagem criada
  for (size_t x = 0; x < img->width; x++)
  {
    for (size_t y = 0; y < img->height; y++)
    {
      ImageSetPixel(newImage, x, y, ImageGetPixel(img, img->width - x - 1, y));
    }
  }

  return newImage;
}

/// Crop a rectangular subimage from img.
/// The rectangle is specified by the top left corner coords (x, y) and
/// width w and height h.
/// Requires:
///   The rectangle must be inside the original image.
/// Ensures:
///   The original img is not modified.
///   The returned image has width w and height h.
///
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
/// On failure, returns NULL and errno/errCause are set accordingly.
Image ImageCrop(Image img, int x, int y, int w, int h)
{ ///
  assert(img != NULL);
  assert(ImageValidRect(img, x, y, w, h));
  
  Image newImage = ImageCreate(w, h, img->maxval);
  
  if (newImage == NULL)
  {
    return NULL;
  }
  //Percorre todos os pixels da imagem e copia os pixels, atribuindo seus valores à nova imagem 
  for (size_t i = 0; i < w; i++)
  {
    for (size_t j = 0; j < h; j++)
    {
      ImageSetPixel(newImage, i, j, ImageGetPixel(img, x + i, y + j));
    }
  }
  
  return newImage;
}

/// Operations on two images

/// Paste an image into a larger image.
/// Paste img2 into position (x, y) of img1.
/// This modifies img1 in-place: no allocation involved.
/// Requires: img2 must fit inside img1 at position (x, y).
void ImagePaste(Image img1, int x, int y, Image img2)
{ ///
  assert(img1 != NULL);
  assert(img2 != NULL);
  assert(ImageValidRect(img1, x, y, img2->width, img2->height));
  
  //Percorre todos os pixels da imagem 2 e os cola na posição exata da imagem 1
  for (size_t i = 0; i < img2->width; i++)
  {
    for (size_t j = 0; j < img2->height; j++)
    {
      ImageSetPixel(img1, x + i, y + j, ImageGetPixel(img2, i, j));
    }

  }

}

/// Blend an image into a larger image.
/// Blend img2 into position (x, y) of img1.
/// This modifies img1 in-place: no allocation involved.
/// Requires: img2 must fit inside img1 at position (x, y).
/// alpha usually is in [0.0, 1.0], but values outside that interval
/// may provide interesting effects.  Over/underflows should saturate.
void ImageBlend(Image img1, int x, int y, Image img2, double alpha)
{ ///
  assert(img1 != NULL);
  assert(img2 != NULL);
  assert(ImageValidRect(img1, x, y, img2->width, img2->height));
  
  //Faz a mesma coisa que o anterior, so que desta vez, ao inves de colar valor do pixel integralmente, 
  //realiza uma operação matematica que faz a imagem 2 parecer translucida sobre a imagem 1
  for (size_t i = 0; i < img2->width; i++)
  {
    for (size_t j = 0; j < img2->height; j++)
    {
      double aux = ImageGetPixel(img1, x + i, y + j) * (1 - alpha) + ImageGetPixel(img2, i, j) * alpha;

      if (aux > img1->maxval)
      {
        ImageSetPixel(img1, x + i, y + j, img1->maxval);
      }
      else if (aux < 0)
      {
        ImageSetPixel(img1, x + i, y + j, 0);
      }
      else
      {
        ImageSetPixel(img1, x + i, y + j, (uint8)round(aux));
      }
    }
  }
}

/// Compare an image to a subimage of a larger image.
/// Returns 1 (true) if img2 matches subimage of img1 at pos (x, y).
/// Returns 0, otherwise.

int ImageMatchSubImage(Image img1, int x, int y, Image img2)
{ ///
  assert(img1 != NULL);
  assert(img2 != NULL);
  assert(ImageValidPos(img1, x, y));
  
  int width = ImageWidth(img2);
  int height = ImageHeight(img2);


  if (!ImageValidRect(img1, x, y, img2->width, img2->height))
  {
    return 0;
  }
  //Percorre todos os pixels da imagem 2 e ve se eles são equivalentes aos pixels da imagem de area(w,h) na 
  //posição(x,y)
  for (size_t i = 0; i < width; i++)
  {
    for (size_t j = 0; j < height; j++)
    {
      LocateComparisons++;//Variável global utilizada para contar as comparações de niveis de cinza
      if (ImageGetPixel(img1, x + i, y + j) != ImageGetPixel(img2, i, j))
      { 
        return 0;
      }
    }
  }
  
  

  
  
  return 1;
}

/// Locate a subimage inside another image.
/// Searches for img2 inside img1.
/// If a match is found, returns 1 and matching position is set in vars (*px, *py).
/// If no match is found, returns 0 and (*px, *py) are left untouched.
//  Lets try parallelize this function
int ImageLocateSubImage(Image img1, int *px, int *py, Image img2)
{ ///
  assert(img1 != NULL);
  assert(img2 != NULL);

  int height1 = ImageHeight(img1);
  int width1 = ImageWidth(img1);
  int height2 = ImageHeight(img2);
  int width2 = ImageWidth(img2);
  
  //Percorre todos os pixels da imagem 1 e utiliza a função anterior para ver se naquela posição, a imagem 2
  //está presente
  for (size_t i = 0; i < width1 - width2; i++)
  {
    for (size_t j = 0; j < height1 - height2; j++)
    {
      LocateComparisons++; //Variável global utilizada para contar as comparações de niveis de cinza
      if (ImageMatchSubImage(img1, i, j, img2))
      {
        *px = i;
        *py = j;
        return 1;
      }
    }
  }

  return 0;
}

/// Filtering

/// Blur an image by a applying a (2dx+1)x(2dy+1) mean filter.
/// Each pixel is substituted by the mean of the pixels in the rectangle
/// [x-dx, x+dx]x[y-dy, y+dy].
/// The image is changed in-place.

void ImageBlur(Image img, int dx, int dy) {
    assert(img != NULL);
    assert(dx >= 0 && dy >= 0);

    
    Image newImage = ImageCreate(img->width, img->height, img->maxval);

    if (newImage == NULL)
    {
      return;
    }

    //Percorre todos os pixels e utiliza o filtro (2dx+1)x(2dy+1) sobre eles, atribuindo seus novoso valores
    //na imagem "newImage", o que certifica que os novos valores nao alteram o calculo para os proximos pixels
    for (size_t i = 0; i < img->width; i++)
    {
      for (size_t j = 0; j < img->height; j++)
      {
        double sum = 0;
        double count = 0;
        int countx = i-dx;
        int county = j-dy;

        if (countx < 0)
        {
          countx = 0;
        }
        if (county < 0)
        {
          county = 0;
        }         
        //Sequencia de loops para calcular a soma
        for (size_t k = countx; k <= i + dx; k++)
        {

          for (size_t l = county; l <= j + dy; l++)
          {
            if (ImageValidPos(img, k, l))
            {
              sum += ImageGetPixel(img, k, l);
              count++;
            }
          }
        }
        uint8 mean = (uint8)round(sum / count); //Calcula a média
        if (mean > img->maxval)
        {
          mean = img->maxval;
        }
        else if (mean < 0)
        {
          mean = 0;
        }
        
        ImageSetPixel(newImage, i, j,mean ); //Aplica o filtro
      }
    }

    //Atribui os valores da imagem nova à imagem antiga e deleta a nova imagem
    for (size_t i = 0; i < img->width; i++)
    {
      for (size_t j = 0; j < img->height; j++)
      {
        ImageSetPixel(img, i, j, ImageGetPixel(newImage, i, j));
      }
    }

    ImageDestroy(&newImage);


}
