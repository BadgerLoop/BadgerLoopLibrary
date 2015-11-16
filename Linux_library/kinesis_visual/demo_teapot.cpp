///////////////////////////////////////////////////////////////////
//                                                               //
// RPI MPU6050 Teapot Demo                                       //
//                                                               //
// Displays the orientation of the sensor in yaw/pitch/roll      //
// on coloured dials and a stripchart and draws a wireframe      //
// projection of a teapot model from a loaded .obj 3d file.      //
//                                                               //
// Ed Butler                                                     //
//                                                               //
///////////////////////////////////////////////////////////////////



#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <termios.h>
#include <poll.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
//#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <linux/fb.h>
#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps20.h"



// class default I2C address is 0x68
// specific I2C addresses may be passed as a parameter here
// AD0 low = 0x68 (default for SparkFun breakout and InvenSense evaluation board)
// AD0 high = 0x69
MPU6050 mpu;


// MPU control/status vars
bool dmpReady = false;  // set true if DMP init was successful
uint8_t mpuIntStatus;   // holds actual interrupt status byte from MPU
uint8_t devStatus;      // return status after each device operation (0 = success, !0 = error)
uint16_t packetSize;    // expected DMP packet size (default is 42 bytes)
uint16_t fifoCount;     // count of all bytes currently in FIFO
uint8_t fifoBuffer[64]; // FIFO storage buffer

// orientation/motion vars
Quaternion q;           // [w, x, y, z]         quaternion container
VectorInt16 aa;         // [x, y, z]            accel sensor measurements
VectorInt16 aaReal;     // [x, y, z]            gravity-free accel sensor measurements
VectorInt16 aaWorld;    // [x, y, z]            world-frame accel sensor measurements
VectorFloat gravity;    // [x, y, z]            gravity vector
float euler[3];         // [psi, theta, phi]    Euler angle container
float ypr[3];           // [yaw, pitch, roll]   yaw/pitch/roll container and gravity vector


/* obj model stuff */

/* Vectors */
typedef float vec3_t[3];
typedef float vec4_t[4];

/* Vertex */
struct obj_vertex_t {
  vec4_t xyzw;
};

/* Texture coordinates */
struct obj_texCoord_t {
  vec3_t uvw;
};

/* Normal vector */
struct obj_normal_t {
  vec3_t ijk;
};

/* Polygon */
struct obj_face_t {
  int type;        /* primitive type */
  int num_elems;      /* number of vertices */
  int *vert_indices;  /* vertex indices */
  int *uvw_indices;   /* texture coordinate indices */
  int *norm_indices;  /* normal vector indices */
};

/* OBJ model structure */
struct obj_model_t {
  int num_verts;                     /* number of vertices */
  int num_texCoords;                 /* number of texture coords. */
  int num_normals;                   /* number of normal vectors */
  int num_faces;                     /* number of polygons */
  int has_texCoords;                 /* has texture coordinates? */
  int has_normals;                   /* has normal vectors? */
  struct obj_vertex_t *vertices;     /* vertex list */
  struct obj_texCoord_t *texCoords;  /* tex. coord. list */
  struct obj_normal_t *normals;      /* normal vector list */
  struct obj_face_t *faces;          /* model's polygons */
};


/*** The master OBJ model which is copied from ***/
struct obj_model_t master_objfile;

/*** An OBJ model ***/
struct obj_model_t objfile;


void cls(void);
void font_init(void);
void fb_init(void);
void fb_putpixel(unsigned int x, unsigned int y, unsigned short int c);
void fb_drawline(int ax, int ay, int bx, int by, int color);
void fb_drawcircle(int xcentre, int ycentre, int radius, int c);
void fb_printfxy(const char *s, int x, int y,int c);
void fb_printchar(char s, int xc, int yc, int cc);
void fb_isoline(float x1,float y1,float z1,float x2,float y2,float z2, unsigned int c) ;
void fb_drawobject(struct obj_model_t *mdl, int colour);

void obj_rotateX(struct obj_model_t *mdl,float angle);
void obj_rotateY(struct obj_model_t *mdl,float angle);
void obj_rotateZ(struct obj_model_t *mdl,float angle);
void obj_copy(struct obj_model_t *srcmdl,struct obj_model_t *dstmdl);
void obj_free (struct obj_model_t *mdl);
int obj_load (const char *filename, struct obj_model_t *mdl);

void quaternion_to_ypr(float *data, Quaternion *q);
void xyz_to_iso_xy(float x, float y, float z, int *isox, int *isoy);


// vars for fb

int fbfd = 0;
struct fb_var_screeninfo vinfo;
struct fb_fix_screeninfo finfo;
long int g_screensize = 0;
char *fbp = 0;
unsigned int g_height;
unsigned int g_width;
int g_isox_off=700;
int g_isoy_off=350;
float g_scale; // obj scale.
char g_objname[100];

// font

FILE *myfont;
#define FONTLEN 12288
static int fontstore[FONTLEN];

// graphics constants

#define RED	 		((0x1f<<11)+(0x00<< 5)+(0x00<< 0))  /* r max */
#define GREEN 	 	((0x00<<11)+(0x3f<< 5)+(0x00<< 0))  /* g max */
#define BLUE	 	((0x00<<11)+(0x00<< 5)+(0x1f<< 0))  /* b max */
#define LT_RED	 	((0x1f<<11)+(0x1f<< 5)+(0x0f<< 0))  
#define LT_GREEN 	((0x0f<<11)+(0x3f<< 5)+(0x0f<< 0))  
#define LT_BLUE	 	((0x0f<<11)+(0x1f<< 5)+(0x1f<< 0))  
#define GREY	 	((0x0f<<11)+(0x1f<< 5)+(0x0f<< 0))
#define	BLACK 		0
#define WHITE 		0xffff


// ================================================================
// ===                      INITIAL SETUP                       ===
// ================================================================

void setup() {
    // initialize device
    printf("Initializing I2C devices...\n");
    mpu.initialize();

    // verify connection
    printf("Testing device connections...\n");
    printf(mpu.testConnection() ? "MPU6050 connection successful\n" : "MPU6050 connection failed\n");

    // load and configure the DMP
    printf("Initializing DMP...\n");
    devStatus = mpu.dmpInitialize();
    
    // make sure it worked (returns 0 if so)
    if (devStatus == 0) {
        // turn on the DMP, now that it's ready
        printf("Enabling DMP...\n");
        mpu.setDMPEnabled(true);

        // enable Arduino interrupt detection
        //Serial.println(F("Enabling interrupt detection (Arduino external interrupt 0)..."));
        //attachInterrupt(0, dmpDataReady, RISING);
        mpuIntStatus = mpu.getIntStatus();

        // set our DMP Ready flag so the main loop() function knows it's okay to use it
        printf("DMP ready!\n");
        dmpReady = true;

        // get expected DMP packet size for later comparison
        packetSize = mpu.dmpGetFIFOPacketSize();
    } else {
        // ERROR!
        // 1 = initial memory load failed
        // 2 = DMP configuration updates failed
        // (if it's going to break, usually the code will be 1)
        printf("DMP Initialization failed (code %d)\n", devStatus);
    }
}


// ================================================================
// ===                    MAIN PROGRAM LOOP                     ===
// ================================================================

void loop() {

	float pitch,roll,yaw;
    char t[100];
    static int x = 0;
    static float new_pitch_x,new_pitch_y,old_pitch_x,old_pitch_y;
    static float new_roll_x,new_roll_y,old_roll_x,old_roll_y;
    static float new_yaw_x,new_yaw_y,old_yaw_x,old_yaw_y;
	int ix1,iy1;
	float tempscal;

    // if programming failed, don't try to do anything
    if (!dmpReady) return;
    // get current FIFO count
    fifoCount = mpu.getFIFOCount();

    if (fifoCount == 1024) {
        // reset so we can continue cleanly
        mpu.resetFIFO();
        printf("FIFO overflow!\n");

    // otherwise, check for DMP data ready interrupt (this should happen frequently)
    } else if (fifoCount >= 42) {

        // read a packet from FIFO
        mpu.getFIFOBytes(fifoBuffer, packetSize);

        // get YPR angles from quaternion in fifo
        mpu.dmpGetQuaternion(&q, fifoBuffer);
		quaternion_to_ypr(ypr, &q); 
		yaw=ypr[0];
		pitch=ypr[1];
		roll=ypr[2];

		// plot stripchart
        fb_drawline (x,50,x,230,BLACK); // blank vertical old pixels
		fb_putpixel(x,(120)+yaw*15,LT_RED);
		fb_putpixel(x,(120)+pitch*15,LT_BLUE);
		fb_putpixel(x,(120)+roll*15,LT_GREEN);
		x++;if(x==320)x=0;

		// label stripchart/dials YPR
		fb_printfxy("   YAW        ", 20,190,LT_RED);
		fb_printfxy("              ", 20,210,BLACK);
		fb_printfxy("              ", 20,230,BLACK);
		sprintf(t,"RAD: %+4.2f", yaw);
		fb_printfxy(t, 20,210,LT_RED);
		sprintf(t,"DEG:%+6.1f", yaw * 180/M_PI);
		fb_printfxy(t, 20,230,LT_RED);

		fb_printfxy("  PITCH       ", 120,190,LT_BLUE);
		fb_printfxy("              ", 120,210,BLACK);
		fb_printfxy("              ", 120,230,BLACK);
		sprintf(t,"RAD: %+4.2f", pitch);
		fb_printfxy(t, 120,210,LT_BLUE);
		sprintf(t,"DEG:%+6.1f", pitch * 180/M_PI);
		fb_printfxy(t, 120,230,LT_BLUE);
		
		fb_printfxy("  ROLL        ", 220,190,LT_GREEN);
		fb_printfxy("              ", 220,210,BLACK);
		fb_printfxy("              ", 220,230,BLACK);
		sprintf(t,"RAD: %+4.2f", roll);
		fb_printfxy(t, 220,210,LT_GREEN);
		sprintf(t,"DEG:%+6.1f", roll * 180/M_PI);
		fb_printfxy(t, 220,230,LT_GREEN);

		// draw dials
		fb_drawcircle (60,300,40,RED);
		new_yaw_x=60+sin(yaw)*40;
		new_yaw_y=300+cos(yaw)*40;
		fb_drawline(60,300,old_yaw_x,old_yaw_y,BLACK);
		fb_drawline(60,300,new_yaw_x,new_yaw_y,LT_RED);
		old_yaw_x=new_yaw_x;
		old_yaw_y=new_yaw_y;

		fb_drawcircle (160,300,40,BLUE);
		new_pitch_x=160+sin(pitch)*40;
		new_pitch_y=300+cos(pitch)*40;
		fb_drawline (160,300,old_pitch_x,old_pitch_y,BLACK);
		fb_drawline (160,300,new_pitch_x,new_pitch_y,LT_BLUE);
		old_pitch_x=new_pitch_x;
		old_pitch_y=new_pitch_y;

		fb_drawcircle (260,300,40,GREEN);
		new_roll_x=260+sin(roll)*40;
		new_roll_y=300+cos(roll)*40;
		fb_drawline (260,300,old_roll_x,old_roll_y,BLACK);
		fb_drawline (260,300,new_roll_x,new_roll_y,LT_GREEN);
		old_roll_x=new_roll_x;
		old_roll_y=new_roll_y;

		// draw iso axes 
		tempscal=g_scale;
		g_scale=300;
		fb_isoline(0,0,0,1,0,0,WHITE);	// X axis
		fb_isoline(0,0,0,0,1,0,WHITE);	// Y axis
		fb_isoline(0,0,0,0,0,1,WHITE);	// Z axis

		// label iso axes 
		xyz_to_iso_xy(1,0,0,&ix1,&iy1);
		fb_printfxy("+x", ix1-16,iy1-6,WHITE);
		xyz_to_iso_xy(0,1,0,&ix1,&iy1);
		fb_printfxy("+y", ix1-5,iy1-13,WHITE);
		xyz_to_iso_xy(0,0,1,&ix1,&iy1);
		fb_printfxy("+z", ix1+1,iy1-1,WHITE);

		// label orientation
		fb_printfxy("Object:",600,10,WHITE);
		fb_printfxy(g_objname,660,10,WHITE);
		g_scale=tempscal;

		// erase old object orientation
		fb_drawobject(&objfile,BLACK);

		// take copy of the master object and rotate the copy
		obj_copy(&master_objfile,&objfile);

		// apply rotations 
		obj_rotateX(&objfile,roll);
		obj_rotateY(&objfile,yaw);
		obj_rotateZ(&objfile,pitch);

		// draw
		fb_drawobject(&objfile,GREY);   
    }
}

void cls (void) {
    system("clear");
}

void fb_init(void) {

    // init fb

    // Open the file for reading and writing
    fbfd = open("/dev/fb0", O_RDWR);
    if (!fbfd) {
	printf("Error: cannot open framebuffer device.\n");
	exit(1);
    }
    printf("The framebuffer device was opened successfully.\n");

    // Get fixed screen information
    if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo)) {
	printf("Error reading fixed information.\n");
	exit(2);
    }
    // Get variable screen information
    if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo)) {
	printf("Error reading variable information.\n");
	exit(3);
    }

    printf("resolution %d x %d, %d bpp\n", vinfo.xres, vinfo.yres, vinfo.bits_per_pixel );
    g_width = vinfo.xres;
    g_height = vinfo.yres;

    // Figure out the size of the screen in bytes
    g_screensize = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;

    // Map the device to memory
    fbp = (char *)mmap(0, g_screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
    if ((int)fbp == -1) {
	printf("Error: failed to map framebuffer device to memory.\n");
	exit(4);
    }
    printf("The framebuffer device was mapped to memory successfully.\n");
}


void font_init(void) {

    int c,a;
	
    // This opens raw bitmap font file  
    if ((myfont = fopen("couriernew10bw.raw", "rb")) == NULL) {
		printf("ERROR The font file was not opened\n");
		exit(7);
    }

    for (a=0; (a<FONTLEN) &&  ((c = getc(myfont)) != EOF); a++) {
		fontstore[a]=c;
    }
    fclose(myfont);		
}

void fb_isoline(float x1,float y1,float z1,float x2,float y2,float z2, unsigned int c) {

	int ix1,ix2,iy1,iy2;
	
	xyz_to_iso_xy(x1,y1,z1,&ix1,&iy1);
	xyz_to_iso_xy(x2,y2,z2,&ix2,&iy2);
    fb_drawline(ix1,iy1,ix2,iy2,c);
}

void xyz_to_iso_xy(float x, float y, float z, int *isox, int *isoy) {
	// ISOMETRIC projection 50/50
	*isox = (int)(g_isox_off-x*g_scale+z*(g_scale/2.0)),
	*isoy = (int)(g_isoy_off-y*g_scale+z*(g_scale/2.0));

}

void fb_putpixel(unsigned int x, unsigned int y, unsigned short int c) {

    // check bounds and plot else segfault
    if ((x<g_width) &&(y<g_height)) {
		*((unsigned short int*)(fbp + (x+vinfo.xoffset) * (vinfo.bits_per_pixel/8) + (y+vinfo.yoffset) * finfo.line_length)) = c; // plot c 16bit
	}
}

void fb_printfxy(const char *s, int xc, int yc,int cc) {

	int x;
	
	for (x=0;s[x]!=0;x++) {
		fb_printchar(s[x], xc, yc, cc);
		xc += 8;
	}
}

void fb_printchar(char s, int xc, int yc, int cc) {
	
	int x,y,i;
	// only print ascii from 32 to 128
	if ((s<32) || (s>128)) return;

	// array is 12288 long, 256*48 bytes, chars are 8*16 bytes
	s -=32;
	// compute along
	i = (s & 31) * 8;
	// compute down
	i += ((s & 224)*128);

	for (x=0;x<7;x++) {
		for (y=0;y<16;y++) {			
			if (s==0) fb_putpixel(xc+x,yc+y,0); // handle spc
			if (fontstore[i+x+256*y] == 0) fb_putpixel(xc+x,yc+y,cc);
		
		}
	}
}



void fb_drawline(int ax, int ay, int bx, int by, int color) {

	int dX = abs(bx-ax);	// store the change in X and Y of the fb_drawline endpoints
	int dY = abs(by-ay);
	int Xincr, Yincr;
	int Yoff=ay;		// speedup plot pixel 

	if (ax > bx) { Xincr=-1; } else { Xincr=1; }			// which direction in X?
	if (ay > by) { Yincr=-1; } else { Yincr=1; }	// which direction in Y?
	if (dX >= dY) {						// if X is the independent variable
		int dPr 	= dY<<1;			// amount to increment decision if right is chosen (always)
		int dPru 	= dPr - (dX<<1);	// amount to increment decision if up is chosen
		int P 		= dPr - dX;			// decision variable start value
		for (; dX>=0; dX--) {           // process each point in the fb_drawline one at a time (just use dX)
			
			fb_putpixel(ax,Yoff,color); 		// plot the pixel

			if (P > 0) {					// is the pixel going right AND up?
				ax+=Xincr;				// increment independent variable
				Yoff+=Yincr;			// increment dependent variable
				P+=dPru;				// increment decision (for up)
			} else {                    // is the pixel just going right?
				ax+=Xincr;				// increment independent variable
				P+=dPr;					// increment decision (for right)
			}
		}		
	} else {							// if Y is the independent variable
		int dPr 	= dX<<1;			// amount to increment decision if right is chosen (always)
		int dPru 	= dPr - (dY<<1);	// amount to increment decision if up is chosen
		int P 		= dPr - dY;			// decision variable start value
		for (; dY>=0; dY--) {           // process each point in the fb_drawline one at a time (just use dY)

			fb_putpixel(ax,Yoff,color); 	// plot the pixel
			
			if (P > 0) {				// is the pixel going up AND right?
				ax+=Xincr;				// increment dependent variable
				Yoff+=Yincr;			// increment independent variable
				P+=dPru;				// increment decision (for up)
			} else {                    // is the pixel just going up?
				Yoff+=Yincr;			// increment independent variable
				P+=dPr;					// increment decision (for right)
			}
		}		
	}		
}

void fb_circlepoints(int cx, int cy, int x, int y, int c) {
        
    if (x == 0) {
        fb_putpixel(cx, cy + y,c);
        fb_putpixel(cx, cy - y,c);
        fb_putpixel(cx + y, cy,c);
        fb_putpixel(cx - y, cy,c);
    } else {
		if (x == y) {
			fb_putpixel(cx + x, cy + y,c);
			fb_putpixel(cx - x, cy + y,c);
			fb_putpixel(cx + x, cy - y,c);
			fb_putpixel(cx - x, cy - y,c);
		} else 
		if (x < y) {
			fb_putpixel(cx + x, cy + y,c);
			fb_putpixel(cx - x, cy + y,c);
			fb_putpixel(cx + x, cy - y,c);
			fb_putpixel(cx - x, cy - y,c);
			fb_putpixel(cx + y, cy + x,c);
			fb_putpixel(cx - y, cy + x,c);
			fb_putpixel(cx + y, cy - x,c);
			fb_putpixel(cx - y, cy - x,c);
		}
	}
}

void fb_drawcircle(int xcentre, int ycentre, int radius, int c) {

    int x = 0;
    int y = radius;
    int p = (5 - radius*4)/4;

    fb_circlepoints(xcentre, ycentre, x, y, c);
    while (x < y) {
        x++;
        if (p < 0) {
            p += 2*x+1;
        } else {
            y--;
            p += 2*(x-y)+1;
        }
        fb_circlepoints(xcentre, ycentre, x, y, c);
    }
}


void obj_rotateX(struct obj_model_t *mdl,float angle) {
    
	int i;
    float newx,newy,newz;
    float co,si;
    
	co = cos(angle);
    si = sin(angle);
	for (i = 0; i < mdl->num_verts; ++i) {
        newx = mdl->vertices[i].xyzw[0];
        newy = mdl->vertices[i].xyzw[1]*co + mdl->vertices[i].xyzw[2]*si;
        newz = -mdl->vertices[i].xyzw[1]*si + mdl->vertices[i].xyzw[2]*co;      
		mdl->vertices[i].xyzw[0]=newx;
		mdl->vertices[i].xyzw[1]=newy;
		mdl->vertices[i].xyzw[2]=newz;
    }
}

void obj_rotateY(struct obj_model_t *mdl, float angle) {
    
	int i;
    float newx,newy,newz;
    float co,si;
    
	co = cos(angle);
    si = sin(angle);
	for (i = 0; i < mdl->num_verts; ++i) {  
        newx = mdl->vertices[i].xyzw[0]*co - mdl->vertices[i].xyzw[2]*si;
        newy = mdl->vertices[i].xyzw[1];
        newz = mdl->vertices[i].xyzw[0]*si + mdl->vertices[i].xyzw[2]*co;
		mdl->vertices[i].xyzw[0]=newx;
		mdl->vertices[i].xyzw[1]=newy;
		mdl->vertices[i].xyzw[2]=newz;
	}
}

void obj_rotateZ(struct obj_model_t *mdl,float angle) {

    int i;
    float newx,newy,newz;
    float co,si;
    
	co = cos(angle);
    si = sin(angle);
	for (i = 0; i < mdl->num_verts; ++i) {  
        newx = mdl->vertices[i].xyzw[0]*co + mdl->vertices[i].xyzw[1]*si;
        newy = -mdl->vertices[i].xyzw[0]*si + mdl->vertices[i].xyzw[1]*co;
        newz = mdl->vertices[i].xyzw[2];
		mdl->vertices[i].xyzw[0]=newx;
		mdl->vertices[i].xyzw[1]=newy;
		mdl->vertices[i].xyzw[2]=newz;
	}
}


void obj_copy(struct obj_model_t *srcmdl,struct obj_model_t *dstmdl) {

	int i,j;
	// copy vertex array
	for (i = 0; i < srcmdl->num_verts; ++i) {
		  dstmdl->vertices[i].xyzw[0] = srcmdl->vertices[i].xyzw[0] ;
		  dstmdl->vertices[i].xyzw[1] = srcmdl->vertices[i].xyzw[1] ;
		  dstmdl->vertices[i].xyzw[2] = srcmdl->vertices[i].xyzw[2] ;
	}
	// copy face array
	for (i = 0; i < srcmdl->num_faces; ++i) {
		for (j = 0; j < srcmdl->faces[i].num_elems; ++j) {
			dstmdl->faces[i].vert_indices[j] = srcmdl->faces[i].vert_indices[j];
		}
	}
}





void quaternion_to_ypr(float *data, Quaternion *q) { 

	float Y,P,R;

	Y = asin( 2*q->x*q->y + 2*q->z*q->w); 
	P = atan2(2*q->y*q->w - 2*q->x*q->z, 1 - 2*q->y*q->y - 2*q->z*q->z); 
	R = atan2(2*q->x*q->w - 2*q->y*q->z, 1 - 2*q->x*q->x - 2*q->z*q->z); 
	if (q->x*q->y + q->z*q->w > 0.4999) {			
		P = (2 * atan2(q->x, q->w));					
		R = 0;											
	} 
	else if (q->x*q->y + q->z*q->w < -0.4999) {		
		P = (-2 * atan2(q->x, q->w));					
		R = 0;											
	} 
	data[0]=Y;		
	data[1]=P;		
	data[2]=R;		
}  




/*
 * obj.c -- obj model loader
 */



/**
 * Free resources allocated for the model.
 */
void obj_free (struct obj_model_t *mdl) {

	int i;

	if (mdl) {
		if (mdl->vertices) {
			free (mdl->vertices);
			mdl->vertices = NULL;
		}
		if (mdl->texCoords) {
			free (mdl->texCoords);
			mdl->texCoords = NULL;
		}
		if (mdl->normals) {
			free (mdl->normals);
			mdl->normals = NULL;
		}
		if (mdl->faces) {
			for (i = 0; i < mdl->num_faces; ++i) {
				if (mdl->faces[i].vert_indices) free (mdl->faces[i].vert_indices);
				if (mdl->faces[i].uvw_indices) free (mdl->faces[i].uvw_indices);
				if (mdl->faces[i].norm_indices) free (mdl->faces[i].norm_indices);
			}
			free (mdl->faces);
			mdl->faces = NULL;
		}
	}
}

/**
 * Allocate resources for the model after first pass.
 */
int obj_alloc (struct obj_model_t *mdl) {

	if (mdl->num_verts) {
		mdl->vertices = (struct obj_vertex_t *)
		malloc (sizeof (struct obj_vertex_t) * mdl->num_verts);
		if (!mdl->vertices)	return 0;
	}
	if (mdl->num_texCoords) {
		mdl->texCoords = (struct obj_texCoord_t *)
		malloc (sizeof (struct obj_texCoord_t) * mdl->num_texCoords);
		if (!mdl->texCoords)	return 0;
	}
	if (mdl->num_normals) {
		mdl->normals = (struct obj_normal_t *)
		malloc (sizeof (struct obj_normal_t) * mdl->num_normals);
		if (!mdl->normals)	return 0;
	}
	if (mdl->num_faces) {
		mdl->faces = (struct obj_face_t *)
		calloc (mdl->num_faces, sizeof (struct obj_face_t));
		if (!mdl->faces)	return 0;
	}
	return 1;
}

/**
 * Load an OBJ model from file -- first pass.
 * Get the number of triangles/vertices/texture coords for
 * allocating buffers.
 */
int obj_load_1 (FILE *fp, struct obj_model_t *mdl) {

	int v, t, n;
	char buf[256];

	while (!feof (fp)) {
		fgets (buf, sizeof (buf), fp);
		switch (buf[0]) {

			case 'v': {
				if (buf[1] == ' ') {				/* Vertex */
					mdl->num_verts++;
				}
				else if (buf[1] == 't') {			/* Texture coords. */
					mdl->num_texCoords++;
				}
				else if (buf[1] == 'n') {			/* Normal vector */
					mdl->num_normals++;
				} else {
					printf ("Warning: unknown token \"%s\"! (ignoring)\n", buf);
				}
				break;
			}

			case 'f': {
				/* Face */
				if (sscanf (buf + 2, "%d/%d/%d", &v, &n, &t) == 3) {
					mdl->num_faces++;
					mdl->has_texCoords = 1;
					mdl->has_normals = 1;
				}
				else if (sscanf (buf + 2, "%d//%d", &v, &n) == 2) {
					mdl->num_faces++;
					mdl->has_texCoords = 0;
					mdl->has_normals = 1;
				}
				else if (sscanf (buf + 2, "%d/%d", &v, &t) == 2) {
					mdl->num_faces++;
					mdl->has_texCoords = 1;
					mdl->has_normals = 0;
				}
				else if (sscanf (buf + 2, "%d", &v) == 1) {
					mdl->num_faces++;
					mdl->has_texCoords = 0;
					mdl->has_normals = 0;
				} else {
					/* Should never be there or the model is very crappy */
					fprintf (stderr, "Error: found face with no vertex!\n");
				}
				break;
			}

			default:
				break;
		}
	}
	/* Check if informations are valid */
	if ((mdl->has_texCoords && !mdl->num_texCoords) || (mdl->has_normals && !mdl->num_normals)) {
		fprintf (stderr, "error: contradiction between collected info!\n");
		return 0;
	}
	if (!mdl->num_verts) {
		fprintf (stderr, "error: no vertex found!\n");
		return 0;
	}
	return 1;
}

/**
 * Load an OBJ model from file -- 2nd pass.
 * This time, read model data and feed buffers.
 */

int obj_load_2 (FILE *fp, struct obj_model_t *mdl) {

	struct obj_vertex_t *pvert = mdl->vertices;
	struct obj_texCoord_t *puvw = mdl->texCoords;
	struct obj_normal_t *pnorm = mdl->normals;
	struct obj_face_t *pface = mdl->faces;
	char buf[128], *pbuf;
	int i;

	while (!feof (fp)) {
		fgets (buf, sizeof (buf), fp);

		switch (buf[0]) {

			case 'v': {
				if (buf[1] == ' ') { /* Vertex */
					if (sscanf (buf + 2, "%f %f %f %f", &pvert->xyzw[0], &pvert->xyzw[1], &pvert->xyzw[2], &pvert->xyzw[3]) != 4) {
						if (sscanf (buf + 2, "%f %f %f", &pvert->xyzw[0],&pvert->xyzw[1], &pvert->xyzw[2] ) != 3) {
							fprintf (stderr, "Error reading vertex data!\n");
							return 0;
						} else {
							pvert->xyzw[3] = 1.0;
						}
					}
					pvert++;
				}
			    else if (buf[1] == 't') {		/* Texture coords. */
					if (sscanf (buf + 2, "%f %f %f", &puvw->uvw[0], &puvw->uvw[1], &puvw->uvw[2]) != 3) {
						if (sscanf (buf + 2, "%f %f", &puvw->uvw[0], &puvw->uvw[1]) != 2) {
							if (sscanf (buf + 2, "%f", &puvw->uvw[0]) != 1) {
								fprintf (stderr, "Error reading texture coordinates!\n");
								return 0;
							} else {
								puvw->uvw[1] = 0.0;
								puvw->uvw[2] = 0.0;
							}
						} else {
							puvw->uvw[2] = 0.0;
						}
					}
					puvw++;
				}
			    else if (buf[1] == 'n') {	/* Normal vector */
					if (sscanf (buf + 2, "%f %f %f", &pnorm->ijk[0], &pnorm->ijk[1], &pnorm->ijk[2]) != 3) {
						fprintf (stderr, "Error reading normal vectors!\n");
						return 0;
					}
					pnorm++;
				}
				break;
			}

			case 'f': {
				pbuf = buf;
				pface->num_elems = 0;

				/* Count number of vertices for this face */
				while (*pbuf) {
					if (*pbuf == ' ') pface->num_elems++;
					pbuf++;
				}

				/* Select primitive type */
				if (pface->num_elems < 3) {
					fprintf (stderr, "Error: a face must have at least 3 vertices!\n");
					return 0;
				}

				/* Memory allocation for vertices */
				pface->vert_indices = (int *)malloc (sizeof (int) * pface->num_elems);
				if (mdl->has_texCoords) pface->uvw_indices = (int *)malloc (sizeof (int) * pface->num_elems);
				if (mdl->has_normals) pface->norm_indices = (int *)malloc (sizeof (int) * pface->num_elems);

				/* Read face data */
				pbuf = buf;
				i = 0;
				for (i = 0; i < pface->num_elems; ++i) {
					pbuf = strchr (pbuf, ' ');
					pbuf++; /* Skip space */

					/* Try reading vertices */
					if (sscanf (pbuf, "%d/%d/%d", &pface->vert_indices[i], &pface->uvw_indices[i], &pface->norm_indices[i]) != 3) {
						if (sscanf (pbuf, "%d//%d", &pface->vert_indices[i], &pface->norm_indices[i]) != 2) {
							if (sscanf (pbuf, "%d/%d", &pface->vert_indices[i], &pface->uvw_indices[i]) != 2) {
								sscanf (pbuf, "%d", &pface->vert_indices[i]);
							}
						}
					}

					/* Indices must start at 0 */
					pface->vert_indices[i]--;
					if (mdl->has_texCoords) pface->uvw_indices[i]--;
					if (mdl->has_normals) pface->norm_indices[i]--;
				}
				pface++;
				break;
			}
		}
	}
	return 1;
}

/**
 * Load an OBJ model from file, in two passes.
 */
int obj_load (const char *filename, struct obj_model_t *mdl) {

	FILE *fp;

	fp = fopen (filename, "r");
	if (!fp) {
		fprintf (stderr, "Error: couldn't open \"%s\"!\n", filename);
		return 0;
	}

	/* reset model data */
	memset (mdl, 0, sizeof (struct obj_model_t));

	/* first pass: read model info */
	if (!obj_load_1 (fp, mdl)) {
		fclose (fp);
		return 0;
	}
	rewind (fp);

	/* memory allocation */
	if (!obj_alloc (mdl)) {
		fclose (fp);
		obj_free (mdl);
		return 0;
	}

	/* second pass: read model data */
	if (!obj_load_2 (fp, mdl)) {
		fclose (fp);
		obj_free (mdl);
		return 0;
	}
	fclose (fp);
	return 1;
}

/**
 * Draw the OBJ model.
 */
void fb_drawobject(struct obj_model_t *mdl, int colour) {

	int i, j;
	float startX,endX;
	float startY,endY;
	float startZ,endZ;

	for (i = 0; i < mdl->num_faces; ++i) {
		startX=(mdl->vertices [mdl->faces[i].vert_indices[0]].xyzw[0]);
		startY=(mdl->vertices [mdl->faces[i].vert_indices[0]].xyzw[1]);
		startZ=(mdl->vertices [mdl->faces[i].vert_indices[0]].xyzw[2]);
		for (j = 0; j < mdl->faces[i].num_elems; ++j) {
			endX=(mdl->vertices [mdl->faces[i].vert_indices[j]].xyzw[0]);
			endY=(mdl->vertices [mdl->faces[i].vert_indices[j]].xyzw[1]);
			endZ=(mdl->vertices [mdl->faces[i].vert_indices[j]].xyzw[2]);
		    fb_isoline(startX,startY,startZ,endX,endY,endZ,colour); 
			startX=endX;
			startY=endY;
			startZ=endZ;
		}
	}
}


int main() {

	if (!obj_load ("teapot.obj", &objfile)) exit (EXIT_FAILURE);
	if (!obj_load ("teapot.obj", &master_objfile)) exit (EXIT_FAILURE);
	g_scale=2.0;
	sprintf(g_objname,"Teapot");
	setup();
	fb_init();	
	font_init();
	cls();
	usleep(100000);
	for (;;) loop();
	obj_free (&objfile);
	obj_free (&master_objfile);
	return 0;
}
