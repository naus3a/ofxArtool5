//
//  ofxARTNftUtils
//
//  Created by nausea on 1/14/16.
//
//

#pragma once
#include "ofMain.h"
#include "jpeglib.h"
#include <AR2/imageFormat.h>

#define JPG_BUFFER_HEIGHT 5

namespace ofxArtool5 {
    
    static void saveSurfaceSetData(AR2SurfaceSetT * sset, string pth){
        const char * cPth = pth.c_str();
        FILE * fp;
        fp = fopen(cPth, "wb");
        
        fwrite(&sset->num, sizeof(int), 1, fp);
        fwrite(&sset->trans1, sizeof(float)*3*4, 1, fp);
        fwrite(&sset->trans2, sizeof(float)*3*4, 1, fp);
        fwrite(&sset->trans3, sizeof(float)*3*4, 1, fp);
        fwrite(&sset->contNum, sizeof(int), 1, fp);
        
        printf("Saving %i surfaces\n",sset->num);
        //surfaces
        for(int i=0;i<sset->num;i++){
            fwrite(&sset->surface[i].trans, sizeof(float)*3*4, 1, fp);
            fwrite(&sset->surface[i].itrans, sizeof(float)*3*4, 1, fp);
            
            //image set
            printf("Surface %i has %i images\n",i,sset->surface[i].imageSet->num);
            fwrite(&sset->surface[i].imageSet->num, sizeof(int), 1, fp);
            for(int j=0;j<sset->surface[i].imageSet->num;j++){
                fwrite(&sset->surface[i].imageSet->scale[j]->xsize, sizeof(int), 1, fp);
                fwrite(&sset->surface[i].imageSet->scale[j]->ysize, sizeof(int), 1, fp);
                fwrite(&sset->surface[i].imageSet->scale[j]->dpi, sizeof(float), 1, fp);
                
                //fwrite(&sset->surface[i].imageSet->scale[j]->imgBW, sizeof(ARUint8)*sset->surface[i].imageSet->scale[j]->xsize*sset->surface[i].imageSet->scale[j]->ysize, 1, fp);
                int nPix = sset->surface[i].imageSet->scale[j]->xsize*sset->surface[i].imageSet->scale[j]->ysize;
                for(int k=0;k<nPix;k++){
                    fwrite(&sset->surface[i].imageSet->scale[j]->imgBW[k], sizeof(ARUint8), 1, fp);
                }
                
                printf("Image %i is %ix%i\n",j,sset->surface[i].imageSet->scale[j]->xsize,sset->surface[i].imageSet->scale[j]->ysize);
            }
            
            //featureset
            printf("Surface %i has %i feature sets\n",i,sset->surface[i].featureSet->num);
            fwrite(&sset->surface[i].featureSet->num, sizeof(int), 1, fp);
            for(int j=0;j<sset->surface[i].featureSet->num;j++){
                printf("Feature set %i has %i coords\n",j,sset->surface[i].featureSet->list[j].num);
                fwrite(&sset->surface[i].featureSet->list[j].num, sizeof(int), 1, fp);
                fwrite(&sset->surface[i].featureSet->list[j].scale, sizeof(int), 1, fp);
                fwrite(&sset->surface[i].featureSet->list[j].maxdpi, sizeof(float), 1, fp);
                fwrite(&sset->surface[i].featureSet->list[j].mindpi, sizeof(float), 1, fp);
                
                for(int k=0;k<sset->surface[i].featureSet->list[j].num;k++){
                    fwrite(&sset->surface[i].featureSet->list[j].coord[k].x, sizeof(int), 1, fp);
                    fwrite(&sset->surface[i].featureSet->list[j].coord[k].y, sizeof(int), 1, fp);
                    fwrite(&sset->surface[i].featureSet->list[j].coord[k].mx, sizeof(float), 1, fp);
                    fwrite(&sset->surface[i].featureSet->list[j].coord[k].my, sizeof(float), 1, fp);
                    fwrite(&sset->surface[i].featureSet->list[j].coord[k].maxSim, sizeof(float), 1, fp);
                }
            }
            
            //markerset
            //fwrite(&sset->surface[i].markerSet->num, sizeof(int), 1, fp);
            //NULL!
        }
        fclose(fp);
    }
    
    static AR2SurfaceSetT * loadSurfaceSetData(string pth){
        const char * cPth = pth.c_str();
        FILE * fp;
        fp=fopen(cPth,"rb");
        if(!fp){
            printf("ERROR: cannot open %s\n",cPth);
            return NULL;
        }
        
        AR2SurfaceSetT * sset;
        arMalloc(sset, AR2SurfaceSetT, 1);
        fread(&sset->num, sizeof(int), 1, fp);
        fread(&sset->trans1, sizeof(float)*3*4, 1, fp);
        fread(&sset->trans2, sizeof(float)*3*4, 1, fp);
        fread(&sset->trans3, sizeof(float)*3*4, 1, fp);
        fread(&sset->contNum, sizeof(int), 1, fp);
        
        printf("Loading %i surfaces\n",sset->num);
        //surfaces
        arMalloc(sset->surface, AR2SurfaceT, sset->num);
        for(int i=0;i<sset->num;i++){
            fread(&sset->surface[i].trans, sizeof(float)*3*4, 1, fp);
            fread(&sset->surface[i].itrans, sizeof(float)*3*4, 1, fp);
            
            //image set
            arMalloc(sset->surface[i].imageSet, AR2ImageSetT, 1);
            fread(&sset->surface[i].imageSet->num, sizeof(int), 1, fp);
            printf("Loading %i images in surface %i\n",sset->surface[i].imageSet->num,i);
            
            sset->surface[i].imageSet->scale = (AR2ImageT**)malloc(sizeof(AR2ImageT*)*sset->surface[i].imageSet->num);
            
            for(int j=0;j<sset->surface[i].imageSet->num;j++){
                int xx,yy;
                float dpi;
                
                fread(&xx, sizeof(int), 1, fp);
                fread(&yy, sizeof(int), 1, fp);
                fread(&dpi, sizeof(float), 1, fp);
                
                sset->surface[i].imageSet->scale[j]=(AR2ImageT*)malloc(sizeof(AR2ImageT));
                sset->surface[i].imageSet->scale[j]->xsize=xx;
                sset->surface[i].imageSet->scale[j]->ysize=yy;
                sset->surface[i].imageSet->scale[j]->dpi=dpi;
                
                sset->surface[i].imageSet->scale[j]->imgBW = (ARUint8*)malloc(sizeof(ARUint8)*xx*yy);
                //fread(sset->surface[i].imageSet->scale[j]->imgBW, sizeof(ARUint8)*xx*yy, 1, fp);
                for(int k=0;k<(xx*yy);k++){
                    fread(&sset->surface[i].imageSet->scale[j]->imgBW[k], sizeof(ARUint8), 1, fp);
                }
                
                printf("Image %i in surface %i is %ix%i\n",j,i,xx,yy);
            }
            
            //feature set
            arMalloc(sset->surface[i].featureSet, AR2FeatureSetT, 1);
            fread(&sset->surface[i].featureSet->num, sizeof(int), 1, fp);
            printf("Loading %i featuresets in surface %i\n",sset->surface[i].featureSet->num,i);
            arMalloc(sset->surface[i].featureSet->list, AR2FeaturePointsT, sset->surface[i].featureSet->num);
            for(int j=0;j<sset->surface[i].featureSet->num;j++){
                fread(&sset->surface[i].featureSet->list[j].num, sizeof(int), 1, fp);
                fread(&sset->surface[i].featureSet->list[j].scale, sizeof(int), 1, fp);
                fread(&sset->surface[i].featureSet->list[j].maxdpi, sizeof(float), 1, fp);
                fread(&sset->surface[i].featureSet->list[j].mindpi, sizeof(float), 1, fp);
                
                printf("Featureset %i in surface %i has %i coords\n",j,i,sset->surface[i].featureSet->list[j].num);
                arMalloc(sset->surface[i].featureSet->list[j].coord, AR2FeatureCoordT, sset->surface[i].featureSet->list[j].num);
                for(int k=0;k<sset->surface[i].featureSet->list[j].num;k++){
                    fread(&sset->surface[i].featureSet->list[j].coord[k].x, sizeof(int), 1, fp);
                    fread(&sset->surface[i].featureSet->list[j].coord[k].y, sizeof(int), 1, fp);
                    fread(&sset->surface[i].featureSet->list[j].coord[k].mx, sizeof(float), 1, fp);
                    fread(&sset->surface[i].featureSet->list[j].coord[k].my, sizeof(float), 1, fp);
                    fread(&sset->surface[i].featureSet->list[j].coord[k].maxSim, sizeof(float), 1, fp);
                }
            }
        }
        
        fclose(fp);
        
        return sset;
    }
    
    struct c_error_mgr{
        struct jpeg_error_mgr pub;
        jmp_buf setjmp_buffer;
    };
    
    typedef struct c_error_mgr * c_error_ptr;
    
    static int cRemoveExt(char * filename){
        int   i, j;
        
        j = -1;
        for( i = 0; filename[i] != '\0'; i++ ) {
            if( filename[i] == '.' ) j = i;
        }
        if( j != -1 ) filename[j] = '\0';
        
        return 0;
    }
    
    static char * cGet_buff(char * buf, int n, FILE * fp){
        char * ret;
        for(;;){
            ret = fgets(buf, n, fp);
            if(ret==NULL)return NULL;
            if(buf[0]!='\n' && buf[0]!='#')return ret;
        }
    }
    
    static void c_error_exit(j_common_ptr cinfo)
    {
        /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
        c_error_ptr myerr = (c_error_ptr) cinfo->err;
        
        /* Always display the message. */
        /* We could postpone this until after returning, if we chose. */
        //(*cinfo->err->output_message) (cinfo);
        
        /* Return control to the setjmp point */
        longjmp(myerr->setjmp_buffer, 1);
    }
    
    static unsigned char * cJpgread(FILE * fp, int *w, int *h, int *nc, float *dpi){
        struct jpeg_decompress_struct    cinfo;
        struct c_error_mgr              jerr;
        unsigned char                    *pixels;
        unsigned char                    *buffer[JPG_BUFFER_HEIGHT];
        int                              bytes_per_line;
        int                              row;
        int                              i;
        int                              ret;
        
        /* Initialize the JPEG decompression object with default error handling. */
        memset(&cinfo, 0, sizeof(cinfo));
        
        /* We set up the normal JPEG error routines, then override error_exit. */
        cinfo.err = jpeg_std_error(&jerr.pub);
        jerr.pub.error_exit = c_error_exit;
        
        /* Establish the setjmp return context for my_error_exit to use. */
        if (setjmp(jerr.setjmp_buffer)) {
            /* If we get here, the JPEG code has signaled an error.
             * We need to clean up the JPEG object, close the input file, and return.
             */
            jpeg_destroy_decompress(&cinfo);
            ARLOGe("Error reading JPEG file.\n");
            return NULL;
        }
        
        jpeg_create_decompress(&cinfo);
    }
    
    static AR2JpegImageT * cAr2ReadJpegImage2(FILE * fp){
        AR2JpegImageT * jpegImage;
        arMalloc( jpegImage, AR2JpegImageT, 1 );
        jpegImage->image = cJpgread(fp, &(jpegImage->xsize), &(jpegImage->ysize), &(jpegImage->nc), &(jpegImage->dpi));
        
        if( jpegImage->image == NULL ) {
            free( jpegImage );
            return NULL;
        }
        return jpegImage;
    }
    
    static AR2ImageSetT * cAr2ReadImageSet(char * filename){
        FILE * fp;
        AR2JpegImageT * jpgImage;
        AR2ImageSetT * iSet;
        float dpi;
        int i,k1;
#if AR2_CAPABLE_ADAPTIVE_TEMPLATE
        int            j, k2;
        ARUint        *p1, *p2;
#endif
        size_t         len;
        const char     ext[] = ".iset";
        char          *buf;
        
        len = strlen(filename) + strlen(ext) + 1; // +1 for nul terminator.
        arMalloc(buf, char, len);
        sprintf(buf, "%s%s", filename, ext);
        ARLOGi("opening file %s%s\n",filename,ext);
        fp = fopen(buf, "rb");
        free(buf);
        if (!fp) {
            ARLOGe("Error: unable to open file '%s%s' for reading.\n", filename, ext);
            return NULL;
        }
        
        arMalloc(iSet, AR2ImageSetT, 1);
        
        if(fread(&(iSet->num), sizeof(iSet->num), 1, fp)!=1 || iSet->num <= 0){
            ARLOGe("Error reading imageSet.\n");
            free(iSet);
            fclose(fp);
            return NULL;
        }
        
        ARLOGi("Imageset contains %d images.\n", iSet->num);
        arMalloc( iSet->scale, AR2ImageT*, iSet->num );
        
        arMalloc(iSet->scale[0], AR2ImageT, 1);
        jpgImage = cAr2ReadJpegImage2(fp);
        //HACK! for some reason (OpenCv?) it cannot read jpeg content...
        //iSet->scale[0]->xsize = 893;
        //iSet->scale[0]->ysize = 1117;
        //iSet->scale[0]->dpi = 120.0;
        
        //missing imgBW!
        
        
        
        fclose(fp);
        
        return iSet;
    }
    
    static AR2SurfaceSetT * cAr2ReadSurfaceSet(const char * filename, const char * ext, ARPattHandle * pattHandle){
            AR2SurfaceSetT * sSet;
            FILE * fp = NULL;
            int readMode;
            char buf[256], name[256];
            int i,j,k;
            
            if( ext == NULL || *ext == '\0' || strcmp(ext,"fset") == 0 ) {
                //No ext or fset
                strncpy(name, filename, sizeof(name) - 1);
                name[sizeof(name) - 1] = '\0';
                readMode = 0;
            }else{
                char namebuf[512];
                sprintf(namebuf, "%s.%s", filename, ext);
                if((fp = fopen(namebuf, "r"))==NULL){
                    ARLOGe("Error opening file '%s': ", filename);
                    ARLOGperror(NULL);
                    return (NULL);
                }
                readMode=1;
            }
            
            arMalloc(sSet, AR2SurfaceSetT, 1);
            
            if(readMode){
                if(cGet_buff(buf, 256, fp)==NULL){
                    fclose(fp);
                    free(sSet);
                    return NULL;
                }
                if(sscanf(buf, "%d", &i)!=1){
                    fclose(fp);
                    free(sSet);
                    return NULL;
                }
                if(i<1){
                    fclose(fp);
                    free(sSet);
                    return NULL;
                }
                sSet->num=i;
                sSet->contNum=0;
            }else{
                sSet->num=1;
                sSet->contNum=0;
            }
            arMalloc(sSet->surface, AR2SurfaceT, sSet->num);
            
            for(i=0;i<sSet->num;i++){
                ARLOGi("\n### Surface No.%d ###\n", i+1);
                if(readMode){
                    if(cGet_buff(buf, 256, fp)==NULL)break;
                    if(sscanf(buf, "%s", name)!=1)break;
                    cRemoveExt( name );
                }
                ARLOGi("  Read ImageSet %s \n",name);
                sSet->surface[i].imageSet = cAr2ReadImageSet(name);
            }
            
            return sSet;
    }
    
}