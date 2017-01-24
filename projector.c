 #include <stdlib.h>
 #include <unistd.h>
 #include <stdio.h>
 #include <fcntl.h>
 #include <linux/fb.h>
 #include <sys/mman.h>
 #include <sys/ioctl.h>

#define arrwidth 1200
#define arrheight 650
#define TIMEDELAY 1

 int main()
 {
     int fbfd = 0;
     struct fb_var_screeninfo vinfo;
     struct fb_fix_screeninfo finfo;
     long int screensize = 0;
     char *fbp = 0;
     int x = 0, y = 0;
     long int location = 0;

    static int mat[arrwidth*arrheight*3];

    char * line = NULL;
    size_t len = 3;
    ssize_t read;
    int i=0;
    int dy=0;



     // Open the file for reading and writing
     fbfd = open("/dev/fb0", O_RDWR);
     if (fbfd == -1) {
         perror("Error: cannot open framebuffer device");
         exit(1);
     }
     printf("The framebuffer device was opened successfully.\n");

     // Get fixed screen information
     if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo) == -1) {
         perror("Error reading fixed information");
         exit(2);
     }

     // Get variable screen information
     if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo) == -1) {
         perror("Error reading variable information");
         exit(3);
     }

     printf("%dx%d, %dbpp\n", vinfo.xres, vinfo.yres, vinfo.bits_per_pixel);

     // Figure out the size of the screen in bytes
     screensize = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;

     // Map the device to memory
     fbp = (char *)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED,
                        fbfd, 0);
     if ((int)fbp == -1) {
         perror("Error: failed to map framebuffer device to memory");
         exit(4);
     }
     printf("The framebuffer device was mapped to memory successfully.\n");

     // Open file 
    FILE *fp = fopen("text.ppm", "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    while ((read = getline(&line, &len, fp)) != -1) {
        mat[i] = atoi(line);
        i++;
    }
    fclose(fp);

    // Fill with black color
     for (y = 10; y < vinfo.yres-10; y++){
         for (x = 20; x < vinfo.xres-20; x++) {
             location = (x+vinfo.xoffset) * (vinfo.bits_per_pixel/8) +
                        (y+vinfo.yoffset) * finfo.line_length;

             if (vinfo.bits_per_pixel == 32) {
                 *(fbp + location) = 0;
                 *(fbp + location+1) = 0;
                 *(fbp + location+2) = 0;
                 *(fbp + location+3) = 0;
             } else  { //assume 16bpp
                 int b = 0;
                 int g = 0;     // A little green
                 int r = 0;    // A lot of red
                 unsigned short int t = r<<11 | g << 5 | b;
                 *((unsigned short int*)(fbp + location)) = t;
             }
         }
     }

    // Project Image to framebuffer
    for(dy = 0 ; dy < 480; dy++){
     for (y = 500; y < 750; y++){
         for (x = 260; x < 991; x++) {
             location = (x+vinfo.xoffset) * (vinfo.bits_per_pixel/8) +
                        (y+vinfo.yoffset-dy) * finfo.line_length;

             if (vinfo.bits_per_pixel == 32) {
                 *(fbp + location) = mat[3*i+0];
                 *(fbp + location+1) = mat[3*i+1];
                 *(fbp + location+2) = mat[3*i+2];
                 *(fbp + location+3) = 0;
             } else  { //assume 16bpp
                 int b = 0;
                 int g = 0;     // A little green
                 int r = 0;    // A lot of red
                 unsigned short int t = r<<11 | g << 5 | b;
                 *((unsigned short int*)(fbp + location)) = t;
             }
              i++;
         }
       usleep(1);

     }
     // Fill animation leftover with black
    for (x = 50; x < 1250; x++) {
        location = (x+vinfo.xoffset) * (vinfo.bits_per_pixel/8) +
                    (y+vinfo.yoffset-dy) * finfo.line_length;

        if (vinfo.bits_per_pixel == 32) {
             *(fbp + location) = 0;
             *(fbp + location+1) = 0;
             *(fbp + location+2) = 0;
             *(fbp + location+3) = 0;
         } else  { //assume 16bpp
             int b = 0;
             int g = 0;     // A little green
             int r = 0;    // A lot of red
             unsigned short int t = r<<11 | g << 5 | b;
             *((unsigned short int*)(fbp + location)) = t;
         }
     }
     i=0;
    }


     munmap(fbp, screensize);
     close(fbfd);
     return 0;
 }
