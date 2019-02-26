//
//  main.c
//  Color_to_gray
//
//  Created by 장지선 on 2/8/19.
//  Copyright © 2019 Geesun. All rights reserved.
//

#include "bmp_format.h"

void fileInfo(BITMAPFILEHEADER *bh, BITMAPINFOHEADER *bi){
    printf("<BITMAP FILE HEADER>\n");
    printf("bfSize: %d\n",bh->bfSize);
    printf("bfOffBits: %d\n",bh->bfOffBits);
    
    printf("<BITMAP INFO HEADER>\n");
    printf("biSize: %d\n",bi->biSize);
    printf("biWidth: %d\n",bi->biWidth);
    printf("biHeight: %d\n",bi->biHeight);
    printf("biPlanes: %d\n",bi->biPlanes);
    printf("biBitCount: %d\n",bi->biBitCount);
    printf("biSizeImage: %d\n",bi->biSizeImage);
    printf("biXpixels: %d\n",bi->biXPelsPerMeter);
    printf("biYpixels: %d\n",bi->biYPelsPerMeter);
    printf("biCompression: %d\n",bi->biCompression); 
    printf("biClrUsed: %d\n",bi->biClrUsed);
    printf("biClrImportant: %d\n",bi->biClrImportant);
    
}

unsigned char* gray_palette_generate(){
    
    unsigned char *palette = (unsigned char*)malloc(sizeof(unsigned char)*GRAY_SCALE_PALETTE);
    unsigned char buffer[4];
    unsigned char* tmp = palette;
    
    buffer[3] = 0x00;
    
    for(int i=0; i<256; i++){
        buffer[0] = i;
        buffer[1] = buffer[0];
        buffer[2] = buffer[0];
        //printf("buf: %x %x %x %x \n",buffer[0],buffer[1],buffer[2],buffer[3]);
        memcpy(tmp, buffer, 4);
        
        //printf("pal1: %x %x %x %x \n", palette[0+i*4],palette[1+i*4],palette[2+i*4],palette[3+i*4]);
        tmp+=4;
    }
    
    return palette;
    
    
}

int main(int argc, const char * argv[]) {
    
    FILE *fpBmp;                    // 비트맵 파일 포인터
    FILE *fpGrayBmp;                // 텍스트 파일 포인터
    BITMAPFILEHEADER fileHeader;    // 비트맵 파일 헤더 구조체 변수
    BITMAPINFOHEADER infoHeader;    // 비트맵 정보 헤더 구조체 변수
    unsigned char* gray_palette;
    
    gray_palette = gray_palette_generate();
    
    //printf("\n");
    unsigned char *image;    // 픽셀 데이터 포인터
    int size;                // 픽셀 데이터 크기
    int width, height;       // 비트맵 이미지의 가로, 세로 크기
    int padding;             // 픽셀 데이터의 가로 크기가 4의 배수가 아닐 때 남는 공간의 크기
    
    
    if(argc!=2){printf("Usage: <original.bmp>");}
    
    char* fileName;
    fileName = (char*)malloc(sizeof(char)*(strlen(argv[1])+10));         //file name initialization
    fileName[0] = '\0';
    strcat(fileName, "gray_");
    strcat(fileName,argv[1]);
    
    
    fpBmp = fopen(argv[1], "rb");    // 비트맵 파일을 바이너리 모드로 열기
    if (fpBmp == NULL)    // 파일 열기에 실패하면
        return 0;         // 프로그램 종료
    
    // 비트맵 파일 헤더 읽기. 읽기에 실패하면 파일 포인터를 닫고 프로그램 종료
    if (fread(&fileHeader, sizeof(BITMAPFILEHEADER), 1, fpBmp) < 1)
    {
        fclose(fpBmp);
        return 0;
    }
    
    // 매직 넘버가 MB가 맞는지 확인(2바이트 크기의 BM을 리틀 엔디언으로 읽었으므로 MB가 됨)
    // 매직 넘버가 맞지 않으면 프로그램 종료
    if (fileHeader.bfType != 'MB')
    {
        fclose(fpBmp);
        return 0;
    }
    
    // 비트맵 정보 헤더 읽기. 읽기에 실패하면 파일 포인터를 닫고 프로그램 종료
    if (fread(&infoHeader, sizeof(BITMAPINFOHEADER), 1, fpBmp) < 1)
    {
        fclose(fpBmp);
        return 0;
    }
    
    fileInfo(&fileHeader, &infoHeader);
    // 24비트 비트맵이 아니면 프로그램 종료
    if (infoHeader.biBitCount != 24)
    {
        printf("Not a 24bit BITMAP\n");
        fclose(fpBmp);
        return 0;
    }
    
    size = infoHeader.biSizeImage;    // 픽셀 데이터 크기
    width = infoHeader.biWidth;       // 비트맵 이미지의 가로 크기
    height = infoHeader.biHeight;     // 비트맵 이미지의 세로 크기
    
    // 이미지의 가로 크기에 픽셀 크기를 곱하여 가로 한 줄의 크기를 구하고 4로 나머지를 구함
    // 그리고 4에서 나머지를 빼주면 남는 공간을 구할 수 있음.
    // 만약 남는 공간이 0이라면 최종 결과가 4가 되므로 여기서 다시 4로 나머지를 구함
    padding = (PIXEL_ALIGN - ((width * PIXEL_SIZE) % PIXEL_ALIGN)) % PIXEL_ALIGN;
    int gray_padding = (PIXEL_ALIGN - ((width * PIXEL_GRAY_SIZE) % PIXEL_ALIGN)) % PIXEL_ALIGN;

    if (size == 0)    // 픽셀 데이터 크기가 0이라면
    {
        // 이미지의 가로 크기 * 픽셀 크기에 남는 공간을 더해주면 완전한 가로 한 줄 크기가 나옴
        // 여기에 이미지의 세로 크기를 곱해주면 픽셀 데이터의 크기를 구할 수 있음
        size = (width * PIXEL_SIZE + padding) * height;
    }
    
    image = malloc(size);    // 픽셀 데이터의 크기만큼 동적 메모리 할당
    
    // 파일 포인터를 픽셀 데이터의 시작 위치로 이동
    fseek(fpBmp, fileHeader.bfOffBits, SEEK_SET);
    
    // 파일에서 픽셀 데이터 크기만큼 읽음. 읽기에 실패하면 파일 포인터를 닫고 프로그램 종료
    if (fread(image, size, 1, fpBmp) < 1)
    {
        fclose(fpBmp);
        return 0;
    }
    
    fclose(fpBmp);    // 비트맵 파일 닫기
    
    
    
    /*--------------------------------------image reading completed------------------------------------------*/
    
    char* gray_pad_buffer;
    gray_pad_buffer = (char*)malloc(sizeof(char)*gray_padding);
    for(int i=0; i<gray_padding;i++){
        gray_pad_buffer[i] = 0x00; 
    }

    printf("output fileName is %s\n",fileName);
    fpGrayBmp = fopen(fileName, "w");    // 결과 출력용 텍스트 파일 열기
    if (fpGrayBmp == NULL)    // 파일 열기에 실패하면
    {
        free(image);      // 픽셀 데이터를 저장한 동적 메모리 해제
        return 0;         // 프로그램 종료
    }
    
    
    infoHeader.biBitCount = 8;
    infoHeader.biSizeImage = (width * PIXEL_GRAY_SIZE + gray_padding) * height;
    infoHeader.biClrUsed = 256; 
    fileHeader.bfOffBits = 1078;
    fileHeader.bfSize = 1078+infoHeader.biSizeImage;


    
    fwrite(&fileHeader, sizeof(BITMAPFILEHEADER), 1, fpGrayBmp);
    fwrite(&infoHeader, sizeof(BITMAPINFOHEADER), 1, fpGrayBmp);
    fwrite(gray_palette, sizeof(unsigned char)*GRAY_SCALE_PALETTE, 1, fpGrayBmp);
    
    for(int i=0; i<height ; i++){
        
        for(int j=0; j<width; j++){
            unsigned char buffer;
            
            int index = 0;
            index = i*(width*PIXEL_SIZE+padding) + j*PIXEL_SIZE;
            
            buffer = image[index+2];
            //buffer.rgbtBlue = image[index];
            //buffer.rgbtGreen = image[index+1];
            //buffer.rgbtRed = image[index+2];
            
            fwrite(&buffer, sizeof(unsigned char), 1, fpGrayBmp);
        }
        fwrite(&gray_pad_buffer, sizeof(char)*gray_padding, 1, fpGrayBmp); //padding을 안넣어서 이 사단이 났다
        
    }
    
    
    fclose(fpGrayBmp);
    
    free(image);
    
    
    
    
    return 0;
}
