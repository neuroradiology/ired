/* Copyleft 2009 -- pancake /at/ nopcode /dot/ org */

static inline char *skipspaces(char *arg) {
	if (*arg==' '||*arg=='\t')
		arg++;
	return arg;
}

#define HEXWIDTH 16
static void hexdump(const unsigned char *buf, unsigned int len) {
	unsigned int i, j;
	for(i=0;i<len;i+=HEXWIDTH) {
		printf("0x%08llx ", seek+i);
		for(j=i;j<i+HEXWIDTH;j++) {
			if (j>=len) {
				printf(j%2?"   ":"  ");
				continue;
			}
			printf("%02x", buf[j]);
			if (j%2) printf(" ");
		}
		for(j=i;j<i+HEXWIDTH;j++) {
			if (j>=len) printf(" ");
			else printf("%c", isprint(buf[j])?buf[j]:'.');
		}
		printf("\n");
	}
}

static void print_fmt(const unsigned char *buf, char *fmt, unsigned int len) {
	char *ofmt = fmt;
	unsigned int i, inc=0, lup=0, up, rep = 0;
	do {
		for(;(*fmt||rep);fmt++) {
			up = rep?rep:*fmt;
			switch(up) {
			case 'i': if (len>3) printf("%d\n", ((buf[0]<<24) | (buf[1]<<16) | (buf[2]<<8) | buf[3])); inc=4; break;
			case 'I': if (len>3) printf("%d\n", ((buf[3]<<24) | (buf[2]<<16) | (buf[1]<<8) | buf[0])); inc=4; break;
			case 's': if (len>1) printf("%d\n", (buf[0]<<8 | buf[1])); inc=2; break;
			case 'S': if (len>1) printf("%d\n", (buf[1]<<8 | buf[0])); inc=2; break;
			case 'o': if (len>0) printf("%oo\n", buf[0]); inc=1; break;
			case 'B': case 'b': if (len>0) printf("0x%02x\n", buf[0]); inc=1; break;
			case 'w': if (len>1) printf("0x%02x%02x\n", buf[1], buf[0]); inc=2; break;
			case 'W': if (len>1) printf("0x%02x%02x\n", buf[0], buf[1]); inc=2; break;
			case 'd': if (len>3) printf("0x%02x%02x%02x%02x\n",buf[3], buf[2], buf[1], buf[0]); inc=4; break;
			case 'D': if (len>3) printf("0x%02x%02x%02x%02x\n", buf[0], buf[1], buf[2], buf[3]); inc=4; break;
			case 'q': if (len>7) printf("0x%02x%02x%02x%02x%02x%02x%02x%02x\n",
				  buf[7], buf[6], buf[5], buf[4], buf[3], buf[2], buf[1], buf[0]); inc=8; break;
			case 'Q': if (len>7) printf("0x%02x%02x%02x%02x%02x%02x%02x%02x\n",
				  buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7]); inc=8; break;
			case '.': inc=1; break;
			case ':': inc=4; break;
			case 'z': for(i=0; inc<len && buf[0] ;i++) { printf("%c", buf[0]); buf++; inc++; } if (i) printf("\n"); break;
			case 'Z': for(i=0; inc<len && buf[0] ;i++) { printf("%c", buf[0]); buf+=2; inc+=2; } if (i) printf("\n"); break;
			case '*': rep = lup; break;
			default: fprintf(stderr, "Unknown format '%c' (%d)\n", up, up);
			}
			if (!rep) lup = up;
			buf += inc;
			if (inc>len) break;
			len -= inc;
		}
		fmt = ofmt;
	} while(!rep && inc && inc <len);
}

static ut64 str2ut64(char *str) {
	ut64 ret = 0LL;
	str = skipspaces(str);
	if (str[0]=='b'&&str[1]==0)
		ret = bsize;
	else if (str[0]=='0'&&str[1]=='x')
		sscanf(str, "0x%llx", &ret);
	else sscanf(str, "%lld", &ret);
	return ret;
}

static int hex2byte(ut8 *val, ut8 c) {
	if ('0' <= c && c <= '9')      *val = (ut8)(*val) * 16 + ( c - '0');
	else if (c >= 'A' && c <= 'F') *val = (ut8)(*val) * 16 + ( c - 'A' + 10);
	else if (c >= 'a' && c <= 'f') *val = (ut8)(*val) * 16 + ( c - 'a' + 10);
	else return 1;
	return 0;
}

/* TODO : cleanup */
static int hexstr2raw(char *arg) {
	unsigned int j, len;
	ut8 *ptr, c, d;
	len = c = d = j = 0;
	for (ptr=(ut8 *)arg;*ptr;ptr++) {
		d = c;
		if (hex2byte(&c, *ptr))
			return -1;
		c |= d;
		if (j++ == 0) c <<= 4;
		if (j==2) {
			arg[len++] = c;
			c = j = 0;
			if (*ptr==' ')
				continue;
		}
	}
	return len;
}

static void *getcurblk(char *arg, unsigned int *len) {
	void *buf;
	if (*arg) {
		*len = (int)str2ut64(arg);
		if (*len <1)
			*len = bsize;
	}
	buf = malloc(*len);
	if (buf == NULL || (io_seek(seek, SEEK_SET)<0)) {
		free(buf);
		buf = NULL;
	} else *len = io_read(buf, *len);
	return buf;
}
