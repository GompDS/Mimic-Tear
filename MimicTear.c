/*
	Mimic Tear v.1.1.0
	Author: Gomp
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <ctype.h>
#include <limits.h>
#include <stdint.h>
#include <stdint.h>

bool int8_ptr_compare(int8_t *ptr1, int8_t *ptr2, int ptr_len);
bool short_ptr_compare(short *ptr1, short *ptr2, int ptr_len);
char *get_next_str(FILE *fp);
int get_next_int(FILE *fp);
void copy_binary(char *src_path, char *dest_path);
void copy_bnd(char *suffix, char *src_path, char *old_id, char *new_id);
void copy_fdp(char *suffix, char *src_path, char *old_id, char *new_id, int game);
void replace_1_byte_binary(char *src_path, char *find, char *replace);
void replace_2_byte_binary(char *src_path, char *find, char *replace);
int find_1_byte_binary(char *src_path, char *find);
int8_t *str_to_int8(char *str);
short *str_to_short(char *str);
void rn_dir_tree(const char *dir_path, const char* find, const char* replace);
void rn_unpacked_files(const char *bnd_type, const char *old_id, const char *new_id);
void unpack_bnd(const char *bnd_type, const char *id, const char *yabber_path);
void repack_bnd(const char *bnd_type, const char *id, const char *yabber_path);
void recomp_bnd(const char *bnd_type, const char *id, const char *yabber_path);
void output_file(char *src, char **outputs, int num_outs, char *path_ext);
void edit_sekiro_fev(const char *old_id, const char *new_id);
void question(void);

int main(int argc, char *argv[])
{
	FILE *fp = fopen("config.ini", "r");
	if(fp == NULL)
	{
		if(fprintf(stderr, "Could not find config.ini") > 0)
		{
			getchar();
		}
		return -1;
	}
	else
	{
		// check for yabber directory
		char *yabber_path = get_next_str(fp);
		char *yabber_dcx_path = (char *) calloc(strlen(yabber_path) + strlen("\\Yabber.DCX.exe"), sizeof(char));
		char *yabber_exe_path = (char *) calloc(strlen(yabber_path) + strlen("\\Yabber.exe"), sizeof(char));
		if(access(yabber_path, F_OK) == -1)
		{
			fprintf(stderr, "%s: \"%s\"\n", "Could not find Yabber directory", yabber_path);
			getchar();
			return -1;
		}
		else
		{
			sprintf(yabber_dcx_path, "%s\\Yabber.DCX.exe", yabber_path);
			if(access(yabber_dcx_path, F_OK) == -1)
			{
				fprintf(stderr, "%s: \"%s\"\n", "Could not find Yabber.DCX.exe", yabber_dcx_path);
				getchar();
				return -1;
			}
			sprintf(yabber_exe_path, "%s\\Yabber.exe", yabber_path);
			if(access(yabber_exe_path, F_OK) == -1)
			{
				fprintf(stderr, "%s: \"%s\"\n", "Could not find Yabber.exe", yabber_exe_path);
				getchar();
				return -1;
			}
		}
		// check for game directory
		char *game_path = get_next_str(fp);
		if(game_path == NULL) return ENOMEM;
		if(access(game_path, F_OK) == -1)
		{
			fprintf(stderr, "%s: \"%s\"\n", "Could not find Game directory", game_path);
			getchar();
			return -1;
		}
		// check for output directory
		char **out_path = (char **) calloc(1, sizeof(char *));
		if(out_path == NULL) return ENOMEM;
		*out_path = get_next_str(fp);
		if(*out_path == NULL) return ENOMEM;
		if(access(*out_path, F_OK) == -1)
		{
			fprintf(stderr, "%s: \"%s\"\n", "Could not find ouput directory", *out_path);
			getchar();
			return -1;
		}
		int num_outs = 1;
		// check for additional output directories
		while(fgetc(fp) == ',')
		{
			out_path = (char **) realloc(out_path, num_outs + 1);
			if(out_path == NULL) return ENOMEM;
			*(out_path + num_outs) = get_next_str(fp);
			if(*(out_path + num_outs) == NULL) return ENOMEM;
			if(access(*(out_path + num_outs), F_OK) == -1)
			{
				fprintf(stderr, "%s: \"%s\"\n", "Could not find ouput directory", *(out_path + num_outs));
				getchar();
				return -1;
			}
			num_outs++;
		}
		// get game
		int game = get_next_int(fp);
		if(game < 0 || game > 1)
		{
			fprintf(stderr, "%s: \"%d\"\n", "Specified game is unknown", game);
			getchar();
			return -1;
		}
		// check if game exists in game path
		if(strstr(game_path, "DARK SOULS III") == NULL && game == 0)
		{
			fprintf(stderr, "The game directory \"%s\" does not match the game setting.\nContinuing could result in incorrect behavior.\n", game_path);
			question();
			getchar();
		}
		if(strstr(game_path, "Sekiro") == NULL && game == 1)
		{
			fprintf(stderr, "The game directory \"%s\" does not match the game setting.\nContinuing could result in incorrect behavior.\n", game_path);
			question();
			getchar();
		}
		// get old id
		char *old_id = get_next_str(fp);
		if(strlen(old_id) > 4)
		{
			fprintf(stderr, "%s: %s\n", "Character ID must be 4 digits", old_id);
			getchar();
			return -1;
		}
		// get new id
		char *new_id = get_next_str(fp);
		if(strlen(new_id) > 4)
		{
			fprintf(stderr, "%s: %s\n", "Character ID must be 4 digits", new_id);
			getchar();
			return -1;
		}
		// get conversion settings
		int setting[7] = {0};
		for(int i = 0; i < 7; i++)
		{
			int value = get_next_int(fp);
			if(value == -1) return ENOMEM;
			setting[i] = value;
		}
		
		if(setting[0])
		{
			copy_bnd("ani", game_path, old_id, new_id);
			unpack_bnd("ani", new_id, yabber_exe_path);
			rn_unpacked_files("ani", old_id, new_id);
			repack_bnd("ani", new_id, yabber_exe_path);
			// output anibnd
			char *src = (char *) calloc(strlen("cxxxx.anibnd.dcx"), sizeof(char));
			sprintf(src, "c%s.anibnd.dcx", new_id);
			output_file(src, out_path, num_outs, "\\chr\\");
		}
		if(setting[1])
		{
			copy_bnd("beh", game_path, old_id, new_id);
			unpack_bnd("beh", new_id, yabber_dcx_path);
			// hex edit old id out of the behbnd
			char *path = (char *) calloc(strlen("cxxxx.behbnd"), sizeof(char));
			sprintf(path, "c%s.behbnd", new_id);
			char *find = (char *) calloc(strlen("cxxxx"), sizeof(char));
			sprintf(find, "c%s", old_id);
			char *replace = (char *) calloc(strlen("cxxxx"), sizeof(char));
			sprintf(replace, "c%s", new_id);
			replace_2_byte_binary(path, find, replace);
			replace_1_byte_binary(path, find, replace);
			recomp_bnd("beh", new_id, yabber_dcx_path);
			// output behbnd
			char *src = (char *) calloc(strlen("cxxxx.behbnd.dcx"), sizeof(char));
			sprintf(src, "c%s.behbnd.dcx", new_id);
			output_file(src, out_path, num_outs, "\\chr\\");
		}
		if(setting[2])
		{
			copy_bnd("chr", game_path, old_id, new_id);
			unpack_bnd("chr", new_id, yabber_exe_path);
			rn_unpacked_files("chr", old_id, new_id);
			// hex edit texture paths in flver
			char *flver_path = calloc(strlen("cxxxx-chrbnd-dcx\\chr\\cxxxx\\cxxxx.flver") + 1, sizeof(char));
			sprintf(flver_path, "c%s-chrbnd-dcx\\chr\\c%s\\c%s.flver", new_id, new_id, new_id);
			char *find = calloc(strlen("\\Model\\chr\\cxxxx\\tex\\") + 1, sizeof(char));
			sprintf(find, "\\Model\\chr\\c%s\\tex\\", old_id);
			char *replace = calloc(strlen("\\Model\\chr\\cxxxx\\tex\\") + 1, sizeof(char));
			sprintf(replace, "\\Model\\chr\\c%s\\tex\\", new_id);
			replace_2_byte_binary(flver_path, find, replace);
			repack_bnd("chr", new_id, yabber_exe_path);
			// output chrbnd
			char *src = (char *) calloc(strlen("cxxxx.chrbnd.dcx"), sizeof(char));
			sprintf(src, "c%s.chrbnd.dcx", new_id);
			output_file(src, out_path, num_outs, "\\chr\\");
		}
		if(setting[3])
		{
			copy_bnd("tex", game_path, old_id, new_id);
			unpack_bnd("tex", new_id, yabber_exe_path);
			rn_unpacked_files("tex", old_id, new_id);
			repack_bnd("tex", new_id, yabber_exe_path);
			// output texbnd
			char *src = (char *) calloc(strlen("cxxxx.texbnd.dcx"), sizeof(char));
			sprintf(src, "c%s.texbnd.dcx", new_id);
			output_file(src, out_path, num_outs, "\\chr\\");
		}
		if(setting[4])
		{
			char *path = (char *) calloc(strlen(game_path) + strlen("\\action\\action\\script\\cxxxx.hks"), sizeof(char));
			sprintf(path, "%s\\action\\script\\cxxxx.hks", game_path);
			char *dest = (char *) calloc(strlen("cxxxx.hks"), sizeof(char));
			sprintf(dest, "c%s.hks", new_id);
			if(access(path, F_OK) == -1)
			{
				fprintf(stderr, "%s: %s\n", "No such file or directory", path);
				getchar();
				exit(-1);
			}
			copy_binary(path, dest);
			// output hks
			char *src = (char *) calloc(strlen("cxxxx.hks"), sizeof(char));
			sprintf(src, "c%s.hks", new_id);
			output_file(src, out_path, num_outs, "\\action\\script\\");
		}
		if(setting[5])
		{
			if(game >= 0 && game <= 1)
			{
				copy_fdp("fev", game_path, old_id, new_id, game);
				char *path;
				char *find;
				char *replace;
				// if ds3
				if(game == 0)
				{
					path = (char *) calloc(strlen("fdp_cxxxx.fev"), sizeof(char));
					sprintf(path, "fdp_c%s.fev", new_id);
					find = (char *) calloc(strlen("fdp_cxxxx"), sizeof(char));
					sprintf(find, "fdp_c%s", old_id);
					replace = (char *) calloc(strlen("fdp_cxxxx"), sizeof(char));
					sprintf(replace, "fdp_c%s", new_id);
					replace_1_byte_binary(path, find, replace);
				}
				// if sekiro
				else
				{
					path = (char *) calloc(strlen("cxxxx.fev"), sizeof(char));
					sprintf(path, "c%s.fev", new_id);
					edit_sekiro_fev(old_id, new_id);
				}
				if(*(old_id + 3) != '0')
				{
					char *rev_old_id = (char *) calloc(strlen(old_id), sizeof(char));
					strcpy(rev_old_id, old_id);
					*(rev_old_id + 3) = '0';
					// if ds3
					if(game == 0)	
					{
						sprintf(find, "fdp_c%s", rev_old_id);
						replace_1_byte_binary(path, find, replace);
					}
					// if sekiro
					else
					{
						edit_sekiro_fev(rev_old_id, new_id);
					}
				}
				// output fev
				char *src;
				if(game == 0)
				{
					src = (char *) calloc(strlen("fdp_cxxxx.fev"), sizeof(char));
					sprintf(src, "fdp_c%s.fev", new_id);
				}
				else
				{
					src = (char *) calloc(strlen("cxxxx.fev"), sizeof(char));
					sprintf(src, "c%s.fev", new_id);
				}
				output_file(src, out_path, num_outs, "\\sound\\");
			}
		}
		if(setting[6])
		{
			if(game >= 0 && game <= 1)
			{
				copy_fdp("fsb", game_path, old_id, new_id, game);
				// output fsb
				char *src;
				if(game == 0)
				{			
					src = (char *) calloc(strlen("fdp_cxxxx.fsb"), sizeof(char));
					sprintf(src, "fdp_c%s.fsb", new_id);
				}
				else
				{
					src = (char *) calloc(strlen("cxxxx.fsb"), sizeof(char));
					sprintf(src, "c%s.fsb", new_id);
				}
				output_file(src, out_path, num_outs, "\\sound\\");
			}
		}
	}
	fclose(fp);
	return 0;
}

bool int8_ptr_compare(int8_t *ptr1, int8_t *ptr2, int ptr_len)
{
	int i = 0;
	while(*(ptr1 + i) == *(ptr2 + i))
	{
		i++;
	}
	if(i < ptr_len)
	{
		return false;
	}
	else
	{
		return true;
	}
}

bool short_ptr_compare(short *ptr1, short *ptr2, int ptr_len)
{
	int i = 0;
	while(*(ptr1 + i) == *(ptr2 + i))
	{
		i++;
	}
	if(i < ptr_len)
	{
		return false;
	}
	else
	{
		return true;
	}
}

char *get_next_str(FILE *fp)
{
	int c = fgetc(fp);
	while(c != '"') 
	{
		c = fgetc(fp);	
	}
	char *path_str = (char *) calloc(1, sizeof(char));
	if(path_str == NULL)
	{
		return NULL;
	}
	c = fgetc(fp);
	int i = 0;
	while(c != '"') 
	{
		*(path_str + i++) = c;
		//reallocate 1 more byte of memory and zero-out unused memory
		path_str = (char *) realloc(path_str, sizeof(char) * (i + 1));
		if(path_str == NULL)
		{
			return NULL;
		}
		memset(path_str + i, 0, sizeof(path_str));
		c = fgetc(fp);
	}
	return path_str;
}

int get_next_int(FILE *fp)
{
	char *value_str = get_next_str(fp);
	if(value_str == NULL) return -1;
	char *endptr;
	int value = strtol(value_str, &endptr, 10);
	if(value > 0) value = 1;
	return value;
}

void copy_binary(char *src_path, char *dest_path)
{
	FILE *src = fopen(src_path, "rb");
	FILE *dest = fopen(dest_path, "wb");
	if(src == NULL)
	{
		exit(-1);
	}
	char buffer = 0; 
	int elements_read = fread(&buffer, sizeof(buffer), 1, src);
	while(elements_read > 0)
	{
		fwrite(&buffer, sizeof(buffer), 1, dest);
		elements_read = fread(&buffer, sizeof(buffer), 1, src);
	}
	fclose(dest);
	fclose(src);
}

void copy_bnd(char *suffix, char *src_path, char *old_id, char *new_id)
{
	char *bnd_src = (char *) calloc(strlen(src_path) + strlen("\\chr\\cxxxx.xxxbnd.dcx"), sizeof(char));
	sprintf(bnd_src, "%s\\chr\\c%s.%sbnd.dcx", src_path, old_id, suffix);
	char *bnd_dest = (char *) calloc(strlen("cxxxx.xxxbnd.dcx"), sizeof(char));
	sprintf(bnd_dest, "c%s.%sbnd.dcx", new_id, suffix);
	if(access(bnd_src, F_OK) == -1)
	{
		fprintf(stderr, "%s: %s\n", "No such file or directory", bnd_src);
		getchar();
		exit(-1);
	}
	copy_binary(bnd_src, bnd_dest);
}

void copy_fdp(char *suffix, char *src_path, char *old_id, char *new_id, int game)
{
	char *fdp_src;
	// if ds3
	if(game == 0)
	{
		fdp_src = (char *) calloc(strlen(src_path) + strlen("\\sound\\fdp_cxxxx.xxx"), sizeof(char));
		sprintf(fdp_src, "%s\\sound\\fdp_c%s.%s", src_path, old_id, suffix);
	}
	// if sekiro
	else if(game == 1)
	{
		fdp_src = (char *) calloc(strlen(src_path) + strlen("\\sound\\cxxxx.xxx"), sizeof(char));
		sprintf(fdp_src, "%s\\sound\\c%s.%s", src_path, old_id, suffix);
	}
	if(access(fdp_src, F_OK) == -1)
	{
		fprintf(stderr, "%s: %s\n", "No such file or directory", fdp_src);
		getchar();
		exit(-1);
	}
	if(access(fdp_src, F_OK) == -1)
	{
		// try with chr id ending in 0 is no result for > 0 last digit
		char *rev_old_id = (char *) calloc(strlen(old_id), sizeof(char));
		strcpy(rev_old_id, old_id);
		*(rev_old_id + 3) = '0';
		// if ds3
		if(game == 0)
		{
			fdp_src = (char *) calloc(strlen(src_path) + strlen("\\sound\\fdp_cxxxx.xxx"), sizeof(char));
			sprintf(fdp_src, "%s\\sound\\fdp_c%s.%s", src_path, rev_old_id, suffix);
		}
		// if sekiro
		else if(game == 1)
		{
			fdp_src = (char *) calloc(strlen(src_path) + strlen("\\sound\\cxxxx.xxx"), sizeof(char));
			sprintf(fdp_src, "%s\\sound\\c%s.%s", src_path, rev_old_id, suffix);
		}
		if(access(fdp_src, F_OK) == -1)
		{
			perror(fdp_src);
			exit(-1);
		}
	}
	char *fdp_dest;
	// if ds3
	if(game == 0)
	{
		fdp_dest = (char *) calloc(strlen("fdp_cxxxx.xxx"), sizeof(char));
		sprintf(fdp_dest, "fdp_c%s.%s", new_id, suffix);
	}
	// if sekiro
	else if(game == 1)
	{
		fdp_dest = (char *) calloc(strlen("cxxxx.xxx"), sizeof(char));
		sprintf(fdp_dest, "c%s.%s", new_id, suffix);
	}
	copy_binary(fdp_src, fdp_dest);
}

void replace_1_byte_binary(char *src_path, char *find, char *replace)
{
	FILE *src = fopen(src_path, "rb");
	if(src == NULL)
	{
		exit(-1);
	}
	FILE *dest = fopen("working", "wb");
	if(strlen(find) != strlen(replace))
	{
		exit(-1);
	}
	int8_t *buffer = (int8_t *) calloc(strlen(find), sizeof(int8_t));
	int8_t *to_find = str_to_int8(find);
	int8_t *to_replace = str_to_int8(replace);
	int max_offset = 0;
	fseek(src, 0L, SEEK_END);
	max_offset = ftell(src);
	fseek(src, 0L, SEEK_SET);
	size_t elements_read = fread(buffer, sizeof(*buffer), 1, src);
	int offset = 0;
	while(elements_read > 0)
	{
		offset += sizeof(int8_t);
		if(*buffer == *to_find)
		{
			if((offset + ((strlen(find) - 1) * sizeof(int8_t))) >= max_offset)
			{
				elements_read = 0;
			}
			else
			{
				elements_read = fread(buffer + 1, sizeof(*buffer), strlen(find) - 1, src);
				offset += (elements_read * sizeof(int8_t));
			}
			if(elements_read == strlen(find) - 1)
			{
				if(int8_ptr_compare(buffer, to_find, strlen(find)))
				{
					fwrite(to_replace, sizeof(*buffer), strlen(find), dest);
				}
				else
				{
					offset -= (elements_read * sizeof(int8_t));
					fseek(src, offset, SEEK_SET); 
					fwrite(buffer, sizeof(*buffer), 1, dest);
				}
			}
			else
			{
				fwrite(buffer, sizeof(*buffer), 1, dest);
			}
		}
		else
		{
			fwrite(buffer, sizeof(*buffer), 1, dest);
		}
		elements_read = fread(buffer, sizeof(*buffer), 1, src);
	}
	fclose(dest);
	fclose(src);
	remove(src_path);
	rename("working", src_path);
}

void replace_2_byte_binary(char *src_path, char *find, char *replace)
{
	FILE *src = fopen(src_path, "rb");
	if(src == NULL)
	{
		exit(-1);
	}
	FILE *dest = fopen("working", "wb");
	if(strlen(find) != strlen(replace))
	{
		exit(-1);
	}
	short *buffer = (short *) calloc(strlen(find), sizeof(short));
	short *to_find = str_to_short(find);
	short *to_replace = str_to_short(replace);
	int max_offset = 0;
	fseek(src, 0L, SEEK_END);
	max_offset = ftell(src);
	fseek(src, 0L, SEEK_SET);
	size_t elements_read = fread(buffer, sizeof(*buffer), 1, src);
	int offset = 0;
	while(elements_read > 0)
	{
		offset += sizeof(short);
		if(*buffer == *to_find)
		{
			if((offset + ((strlen(find) - 1) * sizeof(short))) >= max_offset)
			{
				elements_read = 0;
			}
			else
			{
				elements_read = fread(buffer + 1, sizeof(*buffer), strlen(find) - 1, src);
				offset += (elements_read * sizeof(short));
			}
			if(elements_read == strlen(find) - 1)
			{
				if(short_ptr_compare(buffer, to_find, strlen(find)))
				{
					fwrite(to_replace, sizeof(*buffer), strlen(find), dest);
				}
				else
				{
					offset -= (elements_read * sizeof(short));
					fseek(src, offset, SEEK_SET);
					fwrite(buffer, sizeof(*buffer), 1, dest);
				}
			}
			else
			{
				fwrite(buffer, sizeof(*buffer), 1, dest);
			}
		}
		else
		{
			fwrite(buffer, sizeof(*buffer), 1, dest);
		}
		elements_read = fread(buffer, sizeof(*buffer), 1, src);
	}
	fclose(dest);
	fclose(src);
	remove(src_path);
	rename("working", src_path);
}

int find_1_byte_binary(char *src_path, char *find)
{
	FILE *src = fopen(src_path, "rb");
	if(src == NULL)
	{
		return 1;
	}
	int8_t *buffer = (int8_t *) calloc(strlen(find), sizeof(int8_t));
	int8_t *to_find = str_to_int8(find);
	int i = 0;
	while(i < 4)
	{
		printf("%x ", *(to_find + i));
		i++;
	}
	
	size_t elements_read = fread(buffer, sizeof(*buffer), 1, src);
	while(elements_read > 0)
	{
		if(*buffer == *to_find)
		{
			elements_read = fread(buffer + 1, sizeof(*buffer), strlen(find) - 1, src);
			if(elements_read == strlen(find) - 1)
			{
				if(int8_ptr_compare(buffer, to_find, strlen(find)))
				{
					return 0;
				}
			}
		}
		elements_read = fread(buffer, sizeof(*buffer), 1, src);
	}
	fclose(src);
	return 1;
}

int8_t *str_to_int8(char *str)
{
	int8_t *hex = calloc(strlen(str), sizeof(int8_t));
	for(int i = 0; i < strlen(str); i++)
	{
		char c = *(str + i);
		if(c != '\0')
		{
			if(c == '.') c = 0;
			*(hex + i) = c;
		}
	}
	return hex;
}

short *str_to_short(char *str)
{
	short *hex = calloc(strlen(str), sizeof(short));
	for(int i = 0; i < strlen(str); i++)
	{
		char c = *(str + i);
		if(c == '.') c = 0;
		*(hex + i) = c;
	}
	return hex;
}

void rn_dir_tree(const char *dir_path, const char* find, const char* replace)
{
	char *full_path;
	DIR *dir;
	struct stat stat_path, stat_entry;
	struct dirent *entry;
	// stat for path
	stat(dir_path, &stat_path);

	if ((dir = opendir(dir_path)) == NULL)
	{
		fprintf(stderr, "%s: %s\n", "Can't open directory", dir_path);
		exit(-1);
	}

	while((entry = readdir(dir)) != NULL) 
	{
		if(!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
		{
      continue;
		}
		// get full path of entry
		full_path = calloc(strlen(dir_path) + strlen(entry->d_name) + 2, sizeof(char));
    strcpy(full_path, dir_path);
    strcat(full_path, "\\");
    strcat(full_path, entry->d_name);
		
		// stat for this entry
		stat(full_path, &stat_entry);
		
		// recursion if entry is a directory
		if (S_ISDIR(stat_entry.st_mode) != 0)
		{
			rn_dir_tree(full_path, find, replace);
		}
		
		// go to next entry if entry does not contain the string to find
		if(strstr(entry->d_name, find) == NULL)
		{
			continue;
		}
		
		// rename entry
		char *new_path = calloc(strlen(full_path) + 1, sizeof(char));
		strcpy(new_path, full_path);
		memcpy(strstr(strstr(new_path, entry->d_name), find), replace, 5);
	  rename(full_path, new_path);
		printf("Renamed %s to %s\n", full_path, new_path);
	}
	closedir (dir);
}

void rn_unpacked_files(const char *bnd_type, const char *old_id, const char *new_id)
{
	// rename all files with the old id in directory
	char *path = calloc(strlen("cxxxx-xxxbnd-dcx") + 1, sizeof(char));
	sprintf(path, "c%s-%sbnd-dcx", new_id, bnd_type);
	char *find = calloc(strlen("cxxxx") + 1, sizeof(char));
	sprintf(find, "c%s", old_id);
	char *replace = calloc(strlen("cxxxx") + 1, sizeof(char));
	sprintf(replace, "c%s", new_id);
	rn_dir_tree(path, find, replace);
	// hex edit old id out of yabber xml
	char *xml_path = calloc(strlen(path) + strlen("\\_yabber-bnd4.xml") + 1, sizeof(char));
	sprintf(xml_path, "%s\\_yabber-bnd4.xml", path);
	replace_1_byte_binary(xml_path, find, replace);
}

void unpack_bnd(const char *bnd_type, const char *id, const char *yabber_path)
{
	char *path = calloc(strlen("cxxxx.xxxbnd.dcx") + 1, sizeof(char));
	sprintf(path, "c%s.%sbnd.dcx", id, bnd_type);
	char cwd[PATH_MAX];
	getcwd(cwd, sizeof(cwd));
	char *command = calloc(strlen(yabber_path) + strlen(cwd) + strlen(path) + 10, sizeof(char));
	sprintf(command, "\"%s\" %s\\%s", yabber_path, cwd, path);
	system(command);
	remove(path);
}

void repack_bnd(const char *bnd_type, const char *id, const char *yabber_path)
{
	char *path = calloc(strlen("cxxxx-xxxbnd-dcx") + 1, sizeof(char));
	sprintf(path, "c%s-%sbnd-dcx", id, bnd_type);
	char cwd[PATH_MAX];
	getcwd(cwd, sizeof(cwd));
	char *command = calloc(strlen(yabber_path) + strlen(cwd) + strlen(path) + 10, sizeof(char));
	sprintf(command, "\"%s\" %s\\%s", yabber_path, cwd, path);
	system(command);
	sprintf(command, "rmdir /s /q \"%s\\%s\"", cwd, path);
	system(command);
}

void recomp_bnd(const char *bnd_type, const char *id, const char *yabber_path)
{
	char *path = calloc(strlen("cxxxx.xxxbnd") + 1, sizeof(char));
	sprintf(path, "c%s.%sbnd", id, bnd_type);
	char cwd[PATH_MAX];
	getcwd(cwd, sizeof(cwd));
	char *command = calloc(strlen(yabber_path) + strlen(cwd) + strlen(path) + 10, sizeof(char));
	sprintf(command, "\"%s\" %s\\%s", yabber_path, cwd, path);
	system(command);
	remove(path);
	char *xml_path = calloc(strlen("cxxxx.xxxbnd-yabber-dcx.xml") + 1, sizeof(char));
	sprintf(xml_path, "c%s.%sbnd-yabber-dcx.xml", id, bnd_type);
	remove(xml_path);
}

void output_file(char *src, char **outputs, int num_outs, char *path_ext)
{
	char *dest;
	for(int i = 0; i < num_outs; i++)
	{
		dest = calloc(strlen(*(outputs + i)) + strlen(path_ext) + strlen(src) + 5, sizeof(char));
		sprintf(dest, "%s%s", *(outputs + i), path_ext);
		if(access(dest, F_OK) == -1)
		{
			sprintf(dest, "%s\\%s", *(outputs + i), src);
		}
		else
		{
			sprintf(dest, "%s%s%s", *(outputs + i), path_ext, src);
		}
		copy_binary(src, dest);
		printf("Output %s to %s\n", src, dest);
	}
	remove(src);
}

void edit_sekiro_fev(const char *old_id, const char *new_id)
{
	char *path = (char *) calloc(strlen("cxxxx.fev") + 1, sizeof(char));
	sprintf(path, "c%s.fev", new_id);
	
	char *find = (char *) calloc(strlen("./cxxxx") + 1, sizeof(char));
	sprintf(find, "./c%s", old_id);
	char *replace = (char *) calloc(strlen("./cxxxx") + 1, sizeof(char));
	sprintf(replace, "./c%s", new_id);
	replace_1_byte_binary(path, find, replace);
				
	find = (char *) calloc(strlen(".cxxxx.") + 1, sizeof(char));
	sprintf(find, ".c%s.", old_id);
	replace = (char *) calloc(strlen(".cxxxx.") + 1, sizeof(char));
	sprintf(replace, ".c%s.", new_id);
	replace_1_byte_binary(path, find, replace);
				
	find = (char *) calloc(strlen("bank/cxxxx") + 1, sizeof(char));
	sprintf(find, "bank/c%s", old_id);
	replace = (char *) calloc(strlen("bank/cxxxx") + 1, sizeof(char));
	sprintf(replace, "bank/c%s", new_id);
	replace_1_byte_binary(path, find, replace);
}

void question(void)
{
	char option;

	printf("%s\n", "Are you sure you want to continue? [y/n]");
	scanf("%c", &option);

  if (option == 'n' || option == 'N')
  {
    exit(-1);
  }
  else if(option !=  'y' && option != 'Y')
  {
    printf("Invalid selection.\n");
		getchar();
    question();
  }
	
}