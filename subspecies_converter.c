/*
	Do next: unicode hex edit is finished. Need to add editor encoding hex edit
	Unicode format: 31 00 31 00 37 00 31 00
	Editor format:  31 31 37 31 (only used for behbnd)
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <dirent.h>

#define FILENAME_SIZE 13
#define CHR_ID_LEN 4

bool array_short_compare(short arr1[], short arr2[], short arr_len);
int hex_edit_bnd(char* old_id, char* new_id, char* suffix);
char *get_path(char **path_str, FILE *fp);

int main(int argc, char *argv[])
{
	FILE *fp = fopen("config.ini", "r");
	if(fp == NULL)
	{
		perror("Config file not found.");
		return EXIT_FAILURE;
	}
	else
	{
		char *yabber_path = get_path(&yabber_path, fp);
		puts(yabber_path);
		printf("\n%d", strlen(yabber_path));
		//c = fgetc(fp);	
		char *game_path = get_path(&game_path, fp);
		puts(game_path);
		printf("\n%d", strlen(game_path));
		//char **out_path = malloc(sizeof(char *));
		//*out_path = get_path(out_path, fp);
		//puts(*out_path);
		
		/*
		c = fgetc(fp);
		int path_length = 0;
		FILE *temp = fp;
		while(c != '"')
		{
			path_length++;1
			c = fgetc(++fp);
		}
		fp = temp;
		char *yabber_path = (char*) malloc(sizeof(char) * (path_length + 1));
		fclose(fp);
		puts(yabber_path);*/
	}
	
	return EXIT_SUCCESS;
}

bool array_short_compare(short arr1[], short arr2[], short arr_len)
{
	int i = 0;
	while(arr1[i] == arr2[i])
	{
		i++;
	}
	if(i < arr_len)
	{
		return false;
	}
	else
	{
		return true;
	}
}

int hex_edit_bnd(char* old_id, char* new_id, char* suffix)
{
	char *in_filename = (char *) malloc(FILENAME_SIZE);
	strcpy(in_filename, "c");
	strcat(in_filename, old_id);
	strcat(in_filename, suffix);
	FILE *in = fopen(in_filename, "rb");
	if(in == NULL) return 1;
	
	char *out_filename = (char *) malloc(FILENAME_SIZE);
	strcpy(out_filename, "c");
	strcat(out_filename, new_id);
	strcat(out_filename, suffix);
	FILE *out = fopen(out_filename, "wb");
	
  short buffer[CHR_ID_LEN] = {0};
	
	short old_id_arr[CHR_ID_LEN];
	short new_id_arr[CHR_ID_LEN];
  for(int i = 0; i < CHR_ID_LEN; i++)
	{
		old_id_arr[i] = 0x30 + (*(old_id + i) - '0');
		new_id_arr[i] = 0x30 + (*(new_id + i) - '0');
	}
	
	size_t elements_read = fread(buffer, sizeof(buffer[0]), 1, in);
	while(elements_read > 0)
	{
		if(buffer[0] == old_id_arr[0])
		{
			elements_read = fread(buffer + 1, sizeof(buffer[0]), CHR_ID_LEN - 1, in);
			if(elements_read == CHR_ID_LEN - 1)
			{
				if(array_short_compare(buffer, old_id_arr, CHR_ID_LEN))
				{
					fwrite(new_id_arr, sizeof(buffer[0]), CHR_ID_LEN, out);
				}
				else
				{
					fwrite(buffer, sizeof(buffer[0]), CHR_ID_LEN, out);
				}
			}
			else
			{
				fwrite(buffer, sizeof(buffer[0]), elements_read, out);
			}
		}
		else
		{
			fwrite(buffer, sizeof(buffer[0]), 1, out);
		}
		elements_read = fread(buffer, sizeof(buffer[0]), 1, in);
	}
	fclose(out);
	fclose(in);
}

char *get_path(char **path_str, FILE *fp)
{
	int c = fgetc(fp);
	while(c != '"') 
	{
		c = fgetc(fp);
	}
	*path_str = (char *) malloc(sizeof(char));
	c = fgetc(fp);
	int i = 0;
	while(c != '"') 
	{
		*(*path_str + i++) = c;
		*path_str = (char *) realloc(*path_str, sizeof(char) * (i + 1));
		c = fgetc(fp);
	}
	return *path_str;
}