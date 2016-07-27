//****************************************************************************//
//* Copyright (c) 2016, NVIDIA CORPORATION. All rights reserved.             *//
//*                                                                          *//
//* Redistribution and use in source and binary forms, with or without       *//
//* modification, are permitted provided that the following conditions       *//
//* are met:                                                                 *//
//*  * Redistributions of source code must retain the above copyright        *//
//*    notice, this list of conditions and the following disclaimer.         *//
//*  * Redistributions in binary form must reproduce the above copyright     *//
//*    notice, this list of conditions and the following disclaimer in the   *//
//*    documentation and/or other materials provided with the distribution.  *//
//*  * Neither the name of NVIDIA CORPORATION nor the names of its           *//
//*    contributors may be used to endorse or promote products derived       *//
//*    from this software without specific prior written permission.         *//
//*                                                                          *//
//* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY     *//
//* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE        *//
//* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR       *//
//* PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR        *//
//* CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,    *//
//* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,      *//
//* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR       *//
//* PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY      *//
//* OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT             *//
//* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE    *//
//* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.     *//
//****************************************************************************//
/*
 *  Copyright 2014 NVIDIA Corporation.  All rights reserved.
 *
 *  NOTICE TO USER: The source code, and related code and software
 *  ("Code"), is copyrighted under U.S. and international laws.
 *
 *  NVIDIA Corporation owns the copyright and any patents issued or
 *  pending for the Code.
 *
 *  NVIDIA CORPORATION MAKES NO REPRESENTATION ABOUT THE SUITABILITY
 *  OF THIS CODE FOR ANY PURPOSE.  IT IS PROVIDED "AS-IS" WITHOUT EXPRESS
 *  OR IMPLIED WARRANTY OF ANY KIND.  NVIDIA CORPORATION DISCLAIMS ALL
 *  WARRANTIES WITH REGARD TO THE CODE, INCLUDING NON-INFRINGEMENT, AND
 *  ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 *  PURPOSE.  IN NO EVENT SHALL NVIDIA CORPORATION BE LIABLE FOR ANY
 *  DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES
 *  WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATED TO THE USE OR
 *  PERFORMANCE OF THE CODE, INCLUDING, BUT NOT LIMITED TO, INFRINGEMENT,
 *  LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 *  NEGLIGENCE OR OTHER TORTIOUS ACTION, AND WHETHER OR NOT THE
 *  POSSIBILITY OF SUCH DAMAGES WERE KNOWN OR MADE KNOWN TO NVIDIA
 *  CORPORATION.
 */
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

void blur_gold(const float* input, int height, int width, float* output)
{
  float* blurx = new float[height*width];
  memset(blurx,0,sizeof(float)*height*width);
  for( int i = 0 ; i < 1 ; i++ )
    for( int j = 0 ; j < width  ; j++ )
      blurx[i*width+j] = (input[i*width+j] + 2* input[(i+1)*width+j])/ 3.0;
  for( int i = 1 ; i < height - 1 ; i++ )
    for( int j = 0 ; j < width  ; j++ )
      blurx[i*width+j] = (input[(i-1)*width+j] + input[i*width+j] + input[(i+1)*width+j])/ 3.0;
  for( int i = height-1 ; i < height ; i++ )
    for( int j = 0 ; j < width  ; j++ )
      blurx[i*width+j] = (2*input[(i-1)*width+j] + input[i*width+j] )/ 3.0;
  for( int i = 0 ; i < height ; i++ ){
    for( int j = 0 ; j < 1 ; j++ )
      output[i*width+j] = ( blurx[i*width+j] + 2*blurx[i*width+j+1])/ 3.0;
    for( int j = 1 ; j < width - 1 ; j++ )
      output[i*width+j] = (blurx[i*width+j-1] + blurx[i*width+j] + blurx[i*width+j+1])/ 3.0;
    for( int j = width-1 ; j < width  ; j++ )
      output[i*width+j] = (2*blurx[i*width+j-1] + blurx[i*width+j])/ 3.0;
  }
  delete[] blurx;
}

#define absd(a) ( (a) > 0 ? (a) : (-(a)) )


extern "C" void blur_mirror(float* input, int height, int width, float* output);

int main()
{
  int width = 1000;
  int height = 1200;

  float * input = new float[width*height];
  float * output = new float[width*height];

  for( int i = 0 ; i < height ; i++)
    for( int j = 0 ; j < width ; j++ ){
      input[i*width+j] = (rand()%256)/ 5.0;
      output[i*width+j] = 0.0;
    }


  ///Warmup
  blur_mirror(input,height,width,output);

#ifdef PRINT_OUTPUT
  printf("Input : \n");
  for( int i = 0 ; i < height ; i++ ){
    for( int j = 0 ; j < width ; j++ )
      printf(" %f",input[i*width+j]);
    printf("\n");
  }
  printf("Output : \n");
  for( int i = 0 ; i < height ; i++ ){
    for( int j = 0 ; j < width ; j++ )
      printf(" %f",output[i*width+j]);
    printf("\n");
  }
  printf("Output_Gold : \n");
  for( int i = 0 ; i < height ; i++ ){
    for( int j = 0 ; j < width ; j++ )
      printf(" %f",output_gold[i*width+j]);
    printf("\n");
  }
#endif

  float* output_gold = new float[height*width];
  memset(output_gold,0,sizeof(float)*height*width);
  blur_gold(input,height,width,output_gold);
  double max_error = 0.0;
  double avg_error = 0.0;
  int err_location_i=-1,err_location_j=-1;
  for( int i = 0 ; i < height ; i++ )
    for( int j = 0 ; j < width ; j++ ){
      double curr_error = absd(output[i*width+j] - output_gold[i*width+j]);
      if( curr_error > max_error ){
	max_error = curr_error;
	err_location_i = i;
	err_location_j = j;
      }
      avg_error += curr_error * curr_error;
    }
  avg_error = sqrt(avg_error / (height*width));

  printf("Max Error : %lf at (%d,%d), Average Error: %lf\n",max_error,err_location_i,err_location_j,avg_error);
  delete[] output_gold;
  delete[] input;
  delete[] output;

  if(  avg_error > 1e-5 ) {
    printf("Incorrect Result\n");
    exit(1);
  }
  return 0;
}
