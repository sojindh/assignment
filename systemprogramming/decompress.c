#include <stdio.h>
#include <memory.h>
#include <stdlib.h>
#include <string.h>

#define LENGTH 15
#define BYTES 256
#define MAXSIZE 10000000
#define WINDOWSIZE 4095
#define MAXSIZE 10000000
#define TOTAL_LENGTH 2040949
int  file_sz;



/*어셈블리 함수 구현*/
int add(int a) {
  __asm (
   "mov 0x10(%rbp),%rax;"
      "addq $0x1,%rax;"
      "mov %rbp,%rsp;"
      "pop %rbp;"
      "retq"
    );
}

/*허프만을 위한 연산들과 자료구조*/
int gbg = 0;
extern int file_sz;
unsigned int huffmancode[BYTES][2];

struct huffcode {
	int nbits;
	int code;
};
typedef struct huffcode huffcode_t;

struct huffheap {
	int *h;
	int n, s, cs;
	long *f;
};
typedef struct huffheap heap_t;

/* heap handling funcs */
static heap_t *_heap_create(int s, long *f)
{
	heap_t *h;
	h = malloc(sizeof(heap_t));
	h->h = malloc(sizeof(int)*s);
	h->s = h->cs = s;
	h->n = 0;
	h->f = f;
	return h;
}

static void _heap_destroy(heap_t *heap)
{
	free(heap->h);
	free(heap);
}

#define swap_(I,J) do { int t_; t_ = a[(I)];	\
      a[(I)] = a[(J)]; a[(J)] = t_; } while(0)
static void _heap_sort(heap_t *heap)
{
	int i = 1, j = 2;
	int *a = heap->h;

	while (i < heap->n) {
		if (heap->f[a[i - 1]] >= heap->f[a[i]]) {
			i = j; j++;
		}
		else {
			swap_(i - 1, i);
			i--;
			i = (i == 0) ? j++ : i;
		}
	}
}
#undef swap_

static void _heap_add(heap_t *heap, int c)
{
	if ((heap->n + 1) > heap->s) {
		heap->h = realloc(heap->h, heap->s + heap->cs);
		heap->s += heap->cs;
	}
	heap->h[heap->n] = c;
	heap->n++;
	_heap_sort(heap);
}

static int _heap_remove(heap_t *heap)
{
	if (heap->n > 0) {
		heap->n--;
		return heap->h[heap->n];
	}
	return -1;
}

/*허프만 생성 */
huffcode_t **create_huffman_codes(long *freqs)
{
	char strbit[100];
	huffcode_t **codes;
	heap_t *heap;
	long efreqs[BYTES * 2];
	int preds[BYTES * 2];
	int i, extf = BYTES;
	int r1, r2;

	memcpy(efreqs, freqs, sizeof(long)*BYTES);
	memset(&efreqs[BYTES], 0, sizeof(long)*BYTES);

	heap = _heap_create(BYTES * 2, efreqs);
	if (heap == NULL) return NULL;

	for (i = 0; i < BYTES; i++) if (efreqs[i] > 0) _heap_add(heap, i);

	while (heap->n > 1)
	{
		r1 = _heap_remove(heap);
		r2 = _heap_remove(heap);
		efreqs[extf] = efreqs[r1] + efreqs[r2];
		_heap_add(heap, extf);
		preds[r1] = extf;
		preds[r2] = -extf;
		extf++;
	}
	r1 = _heap_remove(heap);
	preds[r1] = r1;
	_heap_destroy(heap);

	codes = malloc(sizeof(huffcode_t *)*BYTES);

	int bc, bn, ix;
	for (i = 0; i < BYTES; i++) {
		bc = 0; bn = 0;
		if (efreqs[i] == 0) { codes[i] = NULL; continue; }
		ix = i;
		while (abs(preds[ix]) != ix) {
			bc |= ((preds[ix] >= 0) ? 1 : 0) << bn;
			ix = abs(preds[ix]);
			bn++;
		}
		codes[i] = malloc(sizeof(huffcode_t));
		codes[i]->nbits = bn;
		codes[i]->code = bc;
	}


	for (i = 0; i < BYTES; i++) {
		if (codes[i] != NULL) {
			huffmancode[i][0] = codes[i]->code;
			huffmancode[i][1] = codes[i]->nbits;
		}
	}
	return codes;
}

/*허프만 메모리 해제*/
void free_huffman_codes(huffcode_t **c)
{
	int i;

	for (i = 0; i < BYTES; i++) free(c[i]);
	free(c);
}



/*허프만 트리 작성*/
void buildHuffmanTree(unsigned char * buffer, int sz)
{
	huffcode_t **r;
	int i;
	long freqs[BYTES];
	memset(freqs, 0, sizeof freqs);

	for (i = 0; i < sz; i++)
		freqs[buffer[i]] += 1;

	r = create_huffman_codes(freqs);

	free_huffman_codes(r);

}

unsigned char * huffmancompress(unsigned char ** buffer,  int sz)
{

	buildHuffmanTree(*buffer, sz);
	unsigned char* result=malloc(MAXSIZE);
	unsigned int bit_cnt = 0;
	unsigned int bit_input = 0;
	int i = 0;
	int idx = 0;

	for (i = 0; i < file_sz; i++)
	{
		bit_cnt += huffmancode[*(*buffer+i)][1];
		bit_input = ((bit_input) << (huffmancode[*(*buffer + i)][1])) + huffmancode[*(*buffer + i)][0];
		while (bit_cnt >= 8)
		{
			unsigned char x = bit_input >> (bit_cnt - 8);
			result[idx] = x;
			idx++;
			bit_input = bit_input - (x << (bit_cnt - 8));
			bit_cnt -= 8;
		}
	}
	if (bit_cnt != 0)
	{
		unsigned char x = bit_input << (8 - bit_cnt);
		result[idx] = x;
		idx++;
	}
	gbg = 8 - bit_cnt;
	file_sz = idx;
	return result;
}

unsigned char * huffmandecompress(unsigned char ** buffer, unsigned char *original, int original_sz)
{


	buildHuffmanTree(original, original_sz);

	int bit_sz = 0;
	unsigned int pre = 0;
	int cnt = 0;
	unsigned int output = 0;
	unsigned int now = 0;
	unsigned int x = 0;
	int flag = 0;
	int idx = 0;
	int i, j;
	unsigned char *result = malloc(MAXSIZE);//결과를 담을 변수


	for (i = 0; i < file_sz; i++)
	{
		now = *(*buffer + i);
		if (i == file_sz - 1 && gbg != 8)
		{
			now = now >> gbg;
			bit_sz = 8 - gbg;
		}
		else
		{
			bit_sz = 8;
		}
		x = now;
		flag = 0;
		while (bit_sz > 0)
		{
			unsigned int bit = ((x&(1 << bit_sz - 1)) >> bit_sz - 1);
			cnt += 1;
			x = x - (bit << bit_sz - 1);
			output = (output << 1) + bit;
			bit_sz--;
			for (j = 0; j <= 0xFF; j++)
			{
				if (huffmancode[j][0] == output && huffmancode[j][1] == cnt)
				{
					flag = 1;
					result[idx] = j;
					idx++;

					break;
				}
			}
			if (flag == 1) {
				cnt = 0;
				output = 0;
				flag = 0;
			}
		}
		pre = output;
	}
	file_sz=idx;
	free(*buffer);
	return result;
}


unsigned char * changecompress(unsigned char ** buffer)
{
	unsigned char * result = malloc(MAXSIZE);
	int i;
	int bit_sz = 0;
	int idx = 0;
	unsigned char output=0;
	for (i = 0; i < file_sz; i++)
	{
		if (bit_sz == 4)
		{
			result[idx] = output;
			idx++;
			bit_sz = 0;
		}
		if ((*(*buffer + i)) == 'g')
			output = (unsigned char)(output << 2) + 0;
		else if ((*(*buffer + i)) == 't')
			output = (unsigned char)(output << 2) + 1;
		else if ((*(*buffer + i)) == 'a')
			output = (unsigned char)(output << 2) + 2;
		else if ((*(*buffer + i)) == 'c')
			output = (unsigned char)(output << 2) + 3;
		else
			continue;
		bit_sz += 1;
	}

	if (bit_sz != 0)
	{
		result[idx] = output;
		idx++;
		bit_sz = 0;
	}
	file_sz=idx;
	free(*buffer);
	return result;
}
unsigned char * changedecompress(unsigned char **buffer)
{
	unsigned char * result = malloc(MAXSIZE);
	unsigned char x = 0;
	unsigned char now = 0;
	int i = 0, j=0;
	int cnt = 0;
	int idx = 0;
	int total = 0;
	for (i = 0; i < file_sz-1; i++)
	{
		if (cnt == 60)
		{
			result[idx] = '\n';
			idx += 1;
			cnt = 0;
			total += 1;
		}
		x = *(*buffer + i);
		for (j = 6; j >= 0; j -= 2)
		{
			cnt += 1;
			now = (x&(3 << j)) >> j;
			if (now == 0)
				result[idx] = 'g';
			else if (now == 1)
				result[idx] = 't';
			if (now == 2)
				result[idx] = 'a';
			else if (now == 3)
				result[idx] = 'c';
			idx += 1;
			total += 1;
		}
	}
	x = *(*buffer + (file_sz-1));
	int bit_go = TOTAL_LENGTH - total;
	if (bit_go == 3)
		x = x << 2;
	else if (bit_go == 2)
		x = x << 4;
	else if (bit_go == 1)
		x = x << 6;
	for (j = 6; j >= 0; j -= 2)
	{
		if (total == TOTAL_LENGTH)
			break;
		now = (x&(3 << j)) >> j;
		if (now == 0)
			result[idx] = 'g';
		else if (now == 1)
			result[idx] = 't';
		if (now == 2)
			result[idx] = 'a';
		else if (now == 3)
			result[idx] = 'c';
		idx += 1;
		total += 1;
	}
	result[idx] = '\n';
	idx += 1;

	file_sz = idx;
	free(*buffer);
	return result;
}
/*
lz77decompress : 압축 해제
parameter : 압축 해제할 문자열(buffer), 문자열의 크기(file_sz)
반환형:  압축 해제된 문자열
*/

unsigned char * lz77decompress(unsigned const char ** buffer)
{
	int idx = 0;
	unsigned char * result = malloc(MAXSIZE);
	int i = 0, j=0, k=0;
	for (i = 0; i < file_sz; i += 3)
	{
		/*압축된 파일을 (i, j, x)로 분리하는 과정*/
		unsigned short pos_len = ((*(*buffer+i)) << 8) + (*(*buffer + (i+1)));
		unsigned char x = (*(*buffer + (i+2)));
		unsigned short pos = (65520 & pos_len) >> 4;
		unsigned short len = 15 & pos_len;
		/*여기까지*/

		if (pos == 0)
		{
			result[idx] = x;
			idx++;
		}
		else
		{
			for (j = 0, k = idx - pos; j < len; j++, k++)
			{
				result[idx] = result[k];
				idx++;
			}
			result[idx] = x;
			idx++;
		}
	}

	file_sz = idx;
	free(*buffer);
	return result;
}


/*
lz77compress : 압축
parameter : 압축하려는 문자열(buffer), 문자열의 크기(file_sz)
반환형: 압축된 문자열
*/
unsigned char * lz77compress(unsigned const char ** buffer)
{
	/*
		LZ77 algorithm
		1. input 값을 view에 넣는다.
		2. view 버퍼의 첫번째 문자를 포함하는 가장 길게 일치하는 문자열을 window 블럭에서 찾는다.
			(여기에서 pointer는 view의 시작점을 의미한다.)
		3. window에서 찾은 문자열의 첫번째 문자와 pointer와의 거리를 i에 담는다.
		4. 일치하는 문자열의 길이를 j에 담는다.
		5. x=pointer+j번째 문자(char):(찾은 문자열 바로 다음 문자)
		6. (i ,j x)
	*/
	int pointer = 0, i = 0, j = 0;//view 탐색시 사용할 변수들 
	unsigned char x;//다음문자 
	int st = 0, ed = 0;//view의 경계선 
	int match_size;//일치하는 길이 
	unsigned short output;//POS+LEN 
	unsigned char * result = malloc(MAXSIZE);//반환
	int k = 0;
	int flag = 0;
	int idx = 0;


	while (pointer < file_sz - 1)//가장 마지막 문자는 남기기
	{
		i = 0; j = 0;
		match_size = 0;
		x = *(*buffer+(pointer + j));
		flag = 0;
		if (WINDOWSIZE < ed) //window 슬라이딩
		{
			st = ed - WINDOWSIZE;
		}

		/*view 버퍼의 첫번째 문자를 포함하는 가장 길게 일치하는 문자열을 window 블럭에서 찾는다.*/
		for (k = st; k < ed; k=add(k))
		{
			match_size = 0;
			int match_idx = 0;
			int view = pointer;
			for (match_idx = k; match_idx <ed&&view < file_sz - 1;  view++, match_size++, match_idx++)
			{
				if (match_size >= LENGTH)
					break;
				if (*(*buffer+view) != *(*buffer + match_idx))
					break;
			}

			if (match_size > j)
			{
				i = pointer - k;
				j = match_size;
				x = *(*buffer + (pointer + j));
			}
		}
		if (j != 0) {
			if (pointer + j + 1 == file_sz - 1)
			{
				flag = 1;
			}
			pointer = pointer + j + 1;
		}
		else
		{
			if (pointer + 1 == file_sz - 1)
			{
				flag = 1;
			}
			pointer = pointer + 1;
		}
		/*여기까지*/

		ed = pointer;
		output = (unsigned short)(i << 4) + (unsigned short)j;
		result[idx] = (unsigned char)(((unsigned int)output & 65280) >> 8);
		idx++;
		result[idx] = (unsigned char)((unsigned int)output & 255);
		idx++;
		result[idx] = x;
		idx++;		
	}

	if (flag == 1)
	{
		x = *(*buffer + (file_sz -1));
		result[idx] = 0;
		idx++;
		result[idx] = 0;
		idx++;
		result[idx] = x;
		idx++;
	}
	free(*buffer);
	file_sz = idx;
	return result;
}


unsigned char * get_buffer(char * file_in)
{
	unsigned char * result;
	FILE *file = fopen(file_in, "rb");
	if (file == NULL)
	{
		printf("Unable to open %s\n", file_in);
		return;
	}
	/*파일의 크기만큼 버퍼 할당, 버퍼에 값 넣기*/
	fseek(file, 0, SEEK_END);
	file_sz = ftell(file);
	result = malloc(file_sz + 2);
	fseek(file, 0, SEEK_SET);
	fread(result, file_sz, 1, file);
	fclose(file);
	/*여기까지*/

	return result;
}

void write_file(unsigned char *buffer, char *file_out)
{
	FILE *file = fopen(file_out, "wb");
	if (file == NULL)
	{
		printf("Unable to open %s\n", file_out);
		return;
	}

	fwrite(buffer,  file_sz, 1, file);
	fclose(file);
}
int main(int argc, char *argv[])
{
	char ftype[2];
	strncpy(ftype, argv[1], 1);
	unsigned char *buffer;
	unsigned char *result;
	int sz=0;
	
	switch(ftype[0])
	{
		case '1':
			buffer=get_buffer("1.xml");
			buffer=lz77compress(&buffer);
			buffer=lz77compress(&buffer);
			buffer=lz77compress(&buffer);
			buffer=lz77decompress(&buffer);
			buffer=lz77decompress(&buffer);
			buffer=lz77decompress(&buffer);
			write_file(buffer, "1.dcpd");
			break;
		case '2':
			buffer = get_buffer("2.html");
			buffer= lz77compress(&buffer);
			sz = file_sz;
			result = huffmancompress(&buffer, sz);
			result = huffmandecompress(&result, buffer, sz);
			result = lz77decompress(&result);
		write_file(result, "2.dcpd");
			break;
		case '3':
			buffer=get_buffer("3.fna");
			buffer=changecompress(&buffer);
			buffer=changedecompress(&buffer);
			write_file(buffer, "3.dcpd");
			break;
		case '4':
			buffer=get_buffer("4.wav");
			sz=file_sz;
			result=huffmancompress(&buffer, sz);
			result=huffmandecompress(&result, buffer, sz);
			write_file(result, "4.dcpd");
			break;
		case '5':
			buffer=get_buffer("5.bmp");
			buffer=lz77compress(&buffer);
			sz=file_sz;
			result=huffmancompress(&buffer, sz);
			result=huffmandecompress(&result, buffer, sz);
			result=lz77decompress(&result);
			write_file(result, "5.dcpd");
			break;
		default:
			break;		
	}
	return 0;
}


