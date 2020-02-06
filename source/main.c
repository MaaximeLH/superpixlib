#ifndef SUPERPIXMANIP_H
# define SUPERPIXMANIP_H

# define  _GNU_SOURCE
# include <stdint.h>
# include <unistd.h>
# include <string.h>
# include <stdio.h>
# include <stdlib.h>
# include <ctype.h>

typedef enum
{
    false = 0,
    true  = 1,
} bool;

typedef struct pixel_s
{
	uint8_t	red;
	uint8_t	blue;
	uint8_t	green;
} pixel_t;

typedef struct ppm_image_s
{
	size_t	height;
	size_t	width;
	pixel_t	*pixels;
	size_t	length;
	char	magic_number[3];
} ppm_image_t;


ppm_image_t		*ppm_new(const char *path);
pixel_t			*ppm_pixel(const ppm_image_t *img, const size_t i, const size_t j);
bool			img_cmp(const pixel_t *p1, const pixel_t *p2);
#endif

uint8_t	img_blue(const pixel_t *p){
	return p->blue;
}

uint8_t	img_green(const pixel_t *p){
	return p->green;
}

uint8_t	img_red(const pixel_t *p){
	return p->red;
}

bool img_cmp(const pixel_t *p1, const pixel_t *p2){
	if (p1->green != p2->green)
		return false;
	if (p1->red != p2->red)
		return false;
	if (p1->blue != p2->blue)
		return false;
	return true;
}

size_t ppm_height(const ppm_image_t *img){
	return img->height;
}

size_t ppm_width(const ppm_image_t *img){
	return img->width;
}

size_t ppm_length(const ppm_image_t *img){
	return img->length;
}

pixel_t	*ppm_pixel(const ppm_image_t *img, const size_t i, const size_t j){
	return &(img->pixels[i * (img->height) + j]);
}

void skip_comment(char **line, size_t *n, FILE *fd){
	while(getline(line, n, fd), !feof(fd) && **line == '#'){
		free(*line);
		line = NULL;
		n = 0;
	}
}

void print_pixel(pixel_t pixel, size_t i){
	printf("Pixel: %lu\nBleu: %d\tVert: %d\tRouge: %d\n",i, (int)pixel.blue, (int)pixel.green, (int)pixel.red);
}

void get_new_line(char **line, size_t *n, FILE *fd){
	free(*line);
	*n = 0;
	*line = NULL;
	skip_comment(line, n, fd);
}

void ppm_put_header(FILE *fd, ppm_image_t *img){
	char	*line;
	size_t	n, i, j;
	char	buffer[10];

	line = NULL;
	n = 0;
	i = 2;
	j = 0;
	skip_comment(&line, &n, fd);

	line[i] = '\0';
	strcpy(img->magic_number,line);
	if (i >= strlen(line)){
		i = 0;
		get_new_line(&line, &n, fd);
	}

	while(isdigit(line[i+j])){
		buffer[j] = line[i+j];
		j++;
	}

	buffer[j] = '\0';
	img->width = (size_t)atoi(buffer);
	i = j + i + 1;

	j = 0;
	if (i >= strlen(line)){
		i = 0;
		get_new_line(&line, &n, fd);
	}
	while(isdigit(line[i+j])){
		buffer[j] = line[i+j];
		j++;
	}
	buffer[j] = '\0';
	img->height = (size_t)atoi(buffer);

	img->length = img->width * img->height;
	if (line[i + j + 1] == '\0' && !strcmp(img->magic_number,"P6")){
		get_new_line(&line, &n, fd);
	}
	free(line);
}

int	ppm_put_data(FILE *fd, ppm_image_t *img){
	size_t	i,j,size;
	pixel_t	*pixel;
	char	*buffer, *ptr_start, *ptr_end;

	
	img->pixels = (pixel_t*)malloc(sizeof(pixel_t) * img->length);
	if (img->pixels == NULL){
		printf("Error: failed to alloc memory.\n");
	}
	if (!strcmp(img->magic_number,"P6")){
		size = img->width*3;
		buffer = (char*)malloc(sizeof(char)*size + 1);
		buffer[size] = '\0';
		for (i = 0; fgets(buffer,size + 1, fd), i < img->height && !feof(fd); i++){
			for (j = 0; j < size; j++){
				pixel = ppm_pixel(img,i,j);
				pixel->red = buffer[3*j];
				pixel->green = buffer[3*j+1];
				pixel->blue = buffer[3*j+2];
				print_pixel(*pixel, 3*j);
			}
		}
	}else{
		buffer = malloc(sizeof(char) * 20);
		for(i = 0 ; i < img->length ; i++){
			fread(buffer, 20, sizeof(char), fd);
			ptr_start = buffer;
			pixel = &(img->pixels[i]);

			while(isspace(*ptr_start))
				ptr_start++;
			ptr_end = ptr_start;
			while(isdigit(*ptr_end))
				ptr_end++;
			*ptr_end = '\0';
			pixel->red = atoi(ptr_start);

			ptr_start = ptr_end+1;
			while(isspace(*ptr_start))
				ptr_start++;
			ptr_end = ptr_start;
			while(isdigit(*ptr_end))
				ptr_end++;
			*ptr_end = '\0';
			pixel->green = atoi(ptr_start);

			ptr_start = ptr_end+1;
			while(isspace(*ptr_start))
				ptr_start++;
			ptr_end = ptr_start;
			while(isdigit(*ptr_end))
				ptr_end++;
			*ptr_end = '\0';
			pixel->blue = atoi(ptr_start);

			print_pixel(*pixel,i);
			ptr_start = ptr_end + 1;
			fseek(fd, -strlen(ptr_start), SEEK_CUR);
		}
	}
	free(buffer);
	return 1;
}

ppm_image_t	*ppm_new(const char *path){
	ppm_image_t	*img;
	FILE		*fd;

	img = malloc(sizeof(ppm_image_t));
	if (img == NULL)
		return NULL;
	fd = fopen(path, "rb");
	if (fd == NULL)
		return NULL;
	ppm_put_header(fd,img);
	ppm_put_data(fd,img);
	fclose(fd);
	return img;
}

int	main(int ac, char **av){
	ppm_image_t	*img;
	size_t		i,j,count;
	pixel_t		*pixel, black;

	black.red = black.blue = black.green = 0;
	if (ac != 2){
		printf("Il manque un argument.\n");
		return 0;
	}
	img = ppm_new(av[1]);
	if (img == NULL){
		printf("Impossible de load %s.\n", av[1]);
		return 0;
	}
	for (count = 0, i = 0 ; i < img->height ; i++){
		for (j = 0; j < img->width ; j++){
			pixel = ppm_pixel(img,i,j);
			if (img_cmp(&black, pixel))
				count++;
		}
	}
	printf("%lu pixels noir dans l'image.\n", count);
	return 0;
}