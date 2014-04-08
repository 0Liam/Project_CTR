#include "lib.h"
#include "utf.h"

// Memory
void char_to_u8_array(unsigned char destination[], char source[], int size, int endianness, int base)
{	
	char tmp[size][2];
    unsigned char *byte_array = malloc(size*sizeof(unsigned char));
	memset(byte_array, 0, size);
	memset(destination, 0, size);
	memset(tmp, 0, size*2);
    
    for (int i = 0; i < size; i ++){
		tmp[i][0] = source[(i*2)];
        tmp[i][1] = source[((i*2)+1)];
		tmp[i][2] = '\0';
        byte_array[i] = (unsigned char)strtol(tmp[i], NULL, base);
    }
	endian_memcpy(destination,byte_array,size,endianness);
	free(byte_array);
}

void endian_memcpy(u8 *destination, u8 *source, u32 size, int endianness)
{ 
    for (u32 i = 0; i < size; i++){
        switch (endianness){
            case(BE):
                destination[i] = source[i];
                break;
            case(LE):
                destination[i] = source[((size-1)-i)];
                break;
        }
    }
}

int CopyData(u8 **dest, u8 *source, u64 size)
{
	if(!*dest){
		*dest = malloc(size);
		if(!*dest) return -1;
	}
	memcpy(*dest,source,size);
	return 0;
}

// Misc
u64 align(u64 value, u64 alignment)
{
	if(value % alignment != 0)
		return value + alignment - value % alignment;
	else
		return value;
}

u64 min_u64(u64 a, u64 b)
{
	if(a < b) return a;
	return b;
}

u64 max_u64(u64 a, u64 b)
{
	if(a > b) return a;
	return b;
}

// Strings
int append_filextention(char *output, u16 max_outlen, char *input, char extention[])
{
	if(output == NULL || input == NULL){
		printf("[!] Memory Error\n");
		return Fail;
	}
	memset(output,0,max_outlen);
	u16 extention_point = strlen(input)+1;
	for(int i = strlen(input)-1; i > 0; i--){
		if(input[i] == '.'){
			extention_point = i;
			break;
		}
	}
	if(extention_point+strlen(extention) >= max_outlen){
		printf("[!] Input File Name Too Large for Output buffer\n");
		return Fail;
	}
	memcpy(output,input,extention_point);
	sprintf(output,"%s%s",output,extention);
	return 0;
}

void memdump(FILE* fout, const char* prefix, const u8* data, u32 size)
{
	u32 i;
	u32 prefixlen = strlen(prefix);
	u32 offs = 0;
	u32 line = 0;
	while(size)
	{
		u32 max = 32;

		if (max > size)
			max = size;

		if (line==0)
			fprintf(fout, "%s", prefix);
		else
			fprintf(fout, "%*s", prefixlen, "");


		for(i=0; i<max; i++)
			fprintf(fout, "%02X", data[offs+i]);
		fprintf(fout, "\n");
		line++;
		size -= max;
		offs += max;
	}
}

int str_u8_to_u16(u16 **dst, u32 *dst_len, u8 *src, u32 src_len)
{
	*dst_len = src_len*sizeof(u16);
	*dst = malloc((*dst_len)+sizeof(u16));
	if(*dst == NULL)
		return -1;
	memset(*dst,0,(*dst_len)+sizeof(u16));
	u16 *tmp = *dst;
	for(int i=0; i<src_len; i++)
		tmp[i] = (u16)src[i];
	return 0;
}

int str_u16_to_u16(u16 **dst, u32 *dst_len, u16 *src, u32 src_len)
{
	*dst_len = src_len*sizeof(u16);
	*dst = malloc((*dst_len)+sizeof(u16));
	if(*dst == NULL)
		return -1;
	memset(*dst,0,(*dst_len)+sizeof(u16));
	u16 *tmp = *dst;
	for(int i=0; i<src_len; i++)
		tmp[i] = src[i];
	return 0;
}

int str_u32_to_u16(u16 **dst, u32 *dst_len, u32 *src, u32 src_len)
{
	*dst_len = src_len*sizeof(u16);
	*dst = malloc((*dst_len)+sizeof(u16));
	if(*dst == NULL)
		return -1;
	memset(*dst,0,(*dst_len)+sizeof(u16));
	u16 *tmp = *dst;
	for(int i=0; i<src_len; i++)
		tmp[i] = (u16)src[i];
	return 0;
}

#ifndef _WIN32
int str_utf8_to_u16(u16 **dst, u32 *dst_len, u8 *src, u32 src_len)
{
	*dst_len = src_len*sizeof(u16);
	*dst = malloc((*dst_len)+sizeof(u16));
	if(*dst == NULL)
		return -1;
	memset(*dst,0,(*dst_len)+sizeof(u16));
	
	UTF16 *target_start = *dst;
	UTF16 *target_end = (target_start + *dst_len);
	
	UTF8 *src_start = (UTF8*)src;
	UTF8 *src_end = (UTF8*)(src+src_len*sizeof(u8));
	
	return ConvertUTF8toUTF16 (&src_start, src_end, &target_start, target_end, strictConversion);
}
#endif

//Char IO
bool DoesFileExist(char *filename)
{
#ifdef _WIN32
	struct _stat64 st;
	return _stat64(filename, &st) == 0;
#else
	struct stat st;
	return stat(filename, &st) == 0;
#endif
}

u64 GetFileSize_u64(char *filename)
{
#ifdef _WIN32
	struct _stat64 st;
	if( _stat64(filename, &st) != 0)
		return 0;
	else
		return st.st_size;
#else
	struct stat st;
	if( stat(filename, &st) != 0)
		return 0;
	else
		return st.st_size;
#endif
}

int makedir(const char* dir)
{
#ifdef _WIN32
	return _mkdir(dir);
#else
	return mkdir(dir, 0777);
#endif
}

char *getcwdir(char *buffer,int maxlen)
{
#ifdef _WIN32
	return _getcwd(buffer,maxlen);
#else
	return getcwd(buffer,maxlen);
#endif
}

int TruncateFile_u64(char *filename, u64 filelen)
{
#ifdef _WIN32
	HANDLE fh;
 
	LARGE_INTEGER fp;
	fp.QuadPart = filelen;
 
	fh = CreateFile(filename, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (fh == INVALID_HANDLE_VALUE) {
		printf("[!] Invalid File handle\n");
		return 1;
	}
 
	if (SetFilePointerEx(fh, fp, NULL, FILE_BEGIN) == 0 || SetEndOfFile(fh) == 0) {
		printf("[!] Truncate failed\n");
		CloseHandle(fh);
		return 1;
	}
 
	CloseHandle(fh);
	return 0;
#else
	return truncate(filename,filelen);
#endif	
}

// Wide Char IO
#ifdef _WIN32
u64 wGetFileSize_u64(u16 *filename)
{
	struct _stat64 st;
	_wstat64((wchar_t*)filename, &st);
	return st.st_size;
}
#endif

//IO Misc
u8* ImportFile(char *file, u64 size)
{
	u64 fsize = GetFileSize_u64(file);
	if(size > 0){
		if(size != fsize){
			fprintf(stderr,"[!] %s has an invalid size (0x%llx)\n",fsize);
			return NULL;
		}
	}

	u8 *data = (u8*)calloc(1,fsize);
	if(!data){
		fprintf(stderr,"[!] Not enough memory\n");
			return NULL;
	}
	FILE *fp = fopen(file,"rb");
	fread(data,fsize,1,fp);
	fclose(fp);

	return data;
}

void WriteBuffer(void *buffer, u64 size, u64 offset, FILE *output)
{
	fseek_64(output,offset);
	fwrite(buffer,size,1,output);
} 

void ReadFile_64(void *outbuff, u64 size, u64 offset, FILE *file)
{
	fseek_64(file,offset);
	fread(outbuff,size,1,file);
}

int fseek_64(FILE *fp, u64 file_pos)
{
#ifdef _WIN32
	fpos_t pos = file_pos;
	return fsetpos(fp,&pos);
#else
	return fseeko(fp,file_pos,SEEK_SET);
#endif
}

//Data Size conversion
u16 u8_to_u16(u8 *value, u8 endianness)
{
	u16 new_value;
	switch(endianness){
		case(BE): new_value =  (value[1]<<0) | (value[0]<<8); break;
		case(LE): new_value = (value[0]<<0) | (value[1]<<8); break;
	}
	return new_value;
}

u32 u8_to_u32(u8 *value, u8 endianness)
{
	u32 new_value;
	switch(endianness){
		case(BE): new_value = (value[3]<<0) | (value[2]<<8) | (value[1]<<16) | (value[0]<<24); break;
		case(LE): new_value = (value[0]<<0) | (value[1]<<8) | (value[2]<<16) | (value[3]<<24); break;
	}
	return new_value;
}


u64 u8_to_u64(u8 *value, u8 endianness)
{
	u64 u64_return = 0;
	switch(endianness){
		case(BE): 
			u64_return |= (u64)value[7]<<0;
			u64_return |= (u64)value[6]<<8;
			u64_return |= (u64)value[5]<<16;
			u64_return |= (u64)value[4]<<24;
			u64_return |= (u64)value[3]<<32;
			u64_return |= (u64)value[2]<<40;
			u64_return |= (u64)value[1]<<48;
			u64_return |= (u64)value[0]<<56;
			break;
			//return (value[7]<<0) | (value[6]<<8) | (value[5]<<16) | (value[4]<<24) | (value[3]<<32) | (value[2]<<40) | (value[1]<<48) | (value[0]<<56);
		case(LE): 
			u64_return |= (u64)value[0]<<0;
			u64_return |= (u64)value[1]<<8;
			u64_return |= (u64)value[2]<<16;
			u64_return |= (u64)value[3]<<24;
			u64_return |= (u64)value[4]<<32;
			u64_return |= (u64)value[5]<<40;
			u64_return |= (u64)value[6]<<48;
			u64_return |= (u64)value[7]<<56;
			break;
			//return (value[0]<<0) | (value[1]<<8) | (value[2]<<16) | (value[3]<<24) | (value[4]<<32) | (value[5]<<40) | (value[6]<<48) | (value[7]<<56);
	}
	return u64_return;
}

int u16_to_u8(u8 *out_value, u16 in_value, u8 endianness)
{
	switch(endianness){
		case(BE):
			out_value[0]=(in_value >> 8);
			out_value[1]=(in_value >> 0);
			break;
		case(LE):
			out_value[0]=(in_value >> 0);
			out_value[1]=(in_value >> 8);
			break;
	}
	return 0;
}

int u32_to_u8(u8 *out_value, u32 in_value, u8 endianness)
{
	switch(endianness){
		case(BE):
			out_value[0]=(in_value >> 24);
			out_value[1]=(in_value >> 16);
			out_value[2]=(in_value >> 8);
			out_value[3]=(in_value >> 0);
			break;
		case(LE):
			out_value[0]=(in_value >> 0);
			out_value[1]=(in_value >> 8);
			out_value[2]=(in_value >> 16);
			out_value[3]=(in_value >> 24);
			break;
	}
	return 0;
}

int u64_to_u8(u8 *out_value, u64 in_value, u8 endianness)
{
	switch(endianness){
		case(BE):
			out_value[0]=(in_value >> 56);
			out_value[1]=(in_value >> 48);
			out_value[2]=(in_value >> 40);
			out_value[3]=(in_value >> 32);
			out_value[4]=(in_value >> 24);
			out_value[5]=(in_value >> 16);
			out_value[6]=(in_value >> 8);
			out_value[7]=(in_value >> 0);
			break;
		case(LE):
			out_value[0]=(in_value >> 0);
			out_value[1]=(in_value >> 8);
			out_value[2]=(in_value >> 16);
			out_value[3]=(in_value >> 24);
			out_value[4]=(in_value >> 32);
			out_value[5]=(in_value >> 40);
			out_value[6]=(in_value >> 48);
			out_value[7]=(in_value >> 56);
			break;
	}
	return 0;
}


