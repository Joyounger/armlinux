/* This file is derived from the GAS 2.1.4 assembler control file.
   The GAS product is under the GNU General Public License, version 2 or later.
   As such, this file is also under that license.

   If the file format changes in the COFF object, this file should be
   subsequently updated to reflect the changes.

   The actual loader module only uses a few of these structures. The full
   set is documented here because I received the full set. If you wish
   more information about COFF, then O'Reilly has a very excellent book.
*/

/*
 * coff-c55x.h
 *
 * Modified from linux/inlcude/coff.h, for TI TSM320c55x
 * by Toshihiro Kobayashi <toshihiro.kobayashi@nokia.com>
 *
 * 2004/07/22:  DSP Gateway version 3.2
 */

#ifndef __COFF_C55X_H
#define __COFF_C55X_H

#define  E_SYMNMLEN  8   /* Number of characters in a symbol name         */
#define  E_FILNMLEN 14   /* Number of characters in a file name           */
#define  E_DIMNUM    4   /* Number of array dimensions in auxiliary entry */

/*
 * These defines are byte order independent. There is no alignment of fields
 * permitted in the structures. Therefore they are declared as characters
 * and the values loaded from the character positions. It also makes it
 * nice to have it "endian" independent.
 */
 
/* Load a short int from the following tables with little-endian formats */
#define COFF_SHORT_L(ps) ((short)(((unsigned short)((unsigned char)ps[1])<<8)|\
				  ((unsigned short)((unsigned char)ps[0]))))

/* Load a 24-bit from the following tables with little-endian formats */
/* T.Koba */
#define COFF_24BIT_L(ps) (((long)(((unsigned long)((unsigned char)ps[2])<<16) |\
				  ((unsigned long)((unsigned char)ps[1])<<8)  |\
				  ((unsigned long)((unsigned char)ps[0])))))
 
/* Load a long int from the following tables with little-endian formats */
#define COFF_LONG_L(ps) (((long)(((unsigned long)((unsigned char)ps[3])<<24) |\
				 ((unsigned long)((unsigned char)ps[2])<<16) |\
				 ((unsigned long)((unsigned char)ps[1])<<8)  |\
				 ((unsigned long)((unsigned char)ps[0])))))
 
/* Load a short int from the following tables with big-endian formats */
#define COFF_SHORT_H(ps) ((short)(((unsigned short)((unsigned char)ps[0])<<8)|\
				  ((unsigned short)((unsigned char)ps[1]))))

/* Load a long int from the following tables with big-endian formats */
/* T.Koba */
#define COFF_24BIT_H(ps) (((long)(((unsigned long)((unsigned char)ps[0])<<16) |\
				  ((unsigned long)((unsigned char)ps[1])<<8)  |\
				  ((unsigned long)((unsigned char)ps[2])))))

/* Load a long int from the following tables with big-endian formats */
#define COFF_LONG_H(ps) (((long)(((unsigned long)((unsigned char)ps[0])<<24) |\
				 ((unsigned long)((unsigned char)ps[1])<<16) |\
				 ((unsigned long)((unsigned char)ps[2])<<8)  |\
				 ((unsigned long)((unsigned char)ps[3])))))

/* T.Koba */
#define COFF_PUT_SHORT_L(val,ps) {\
		ps[0]= val    &0xff;\
		ps[1]=(val>>8)&0xff;\
	}
#define COFF_PUT_24BIT_L(val,ps) {\
		ps[0]= val     &0xff;\
		ps[1]=(val>> 8)&0xff;\
		ps[2]=(val>>16)&0xff;\
	}
#define COFF_PUT_LONG_L(val,ps) {\
		ps[0]= val     &0xff;\
		ps[1]=(val>> 8)&0xff;\
		ps[2]=(val>>16)&0xff;\
		ps[3]=(val>>24)&0xff;\
	}
#define COFF_PUT_SHORT_H(val,ps) {\
		ps[0]=(val>>8)&0xff;\
		ps[1]= val    &0xff;\
	}
#define COFF_PUT_24BIT_H(val,ps) {\
		ps[0]=(val>>16)&0xff;\
		ps[1]=(val>> 8)&0xff;\
		ps[2]= val     &0xff;\
	}
#define COFF_PUT_LONG_H(val,ps) {\
		ps[0]=(val>>24)&0xff;\
		ps[1]=(val>>16)&0xff;\
		ps[2]=(val>> 8)&0xff;\
		ps[3]= val     &0xff;\
	}

/* These may be overridden later by brain dead implementations which generate
   a big-endian header with little-endian data. In that case, generate a
   replacement macro which tests a flag and uses either of the two above
   as appropriate. */

#define COFF_LONG(v)   COFF_LONG_L(v)
#define COFF_24BIT(v)  COFF_24BIT_L(v)
#define COFF_SHORT(v)  COFF_SHORT_L(v)
#define COFF_PUT_LONG(v,p)	COFF_PUT_LONG_L(v,p)
#define COFF_PUT_24BIT(v,p)	COFF_PUT_24BIT_L(v,p)
#define COFF_PUT_SHORT(v,p)	COFF_PUT_SHORT_L(v,p)

/*** coff information for TI TMS320C55x.  */

/********************** FILE HEADER **********************/

struct COFF_filehdr {
	char f_verid[2];	/* version id */
	char f_nscns[2];	/* number of sections		*/
	char f_timdat[4];	/* time & date stamp		*/
	char f_symptr[4];	/* file pointer to symtab	*/
	char f_nsyms[4];	/* number of symtab entries	*/
	char f_opthdr[2];	/* sizeof(optional hdr)		*/
	char f_flags[2];	/* flags			*/
	char f_tgtid[2];	/* target id			*/
};

/*
 *   Bits for f_flags:
 *
 *	F_RELFLG	relocation info stripped from file
 *	F_EXEC		file is executable  (i.e. no unresolved external
 *			references)
 *	F_LNNO		line numbers stripped from file
 *	F_LSYMS		local symbols stripped from file
 *	F_MINMAL	this is a minimal object file (".m") output of fextract
 *	F_UPDATE	this is a fully bound update file, output of ogen
 *	F_SWABD		this file has had its bytes swabbed (in names)
 *	F_AR16WR	this file has the byte ordering of an AR16WR
 *			(e.g. 11/70) machine
 *	F_AR32WR	this file has the byte ordering of an AR32WR machine
 *			(e.g. vax and iNTEL 386)
 *	F_AR32W		this file has the byte ordering of an AR32W machine
 *			(e.g. 3b,maxi)
 *	F_PATCH		file contains "patch" list in optional header
 *	F_NODF		(minimal file only) no decision functions for
 *			replaced functions
 *	F_SYMMERGE	(T.Koba) duplicate symbols were removed.
 */

#define  COFF_F_RELFLG		0000001
#define  COFF_F_EXEC		0000002
#define  COFF_F_LNNO		0000004
#define  COFF_F_LSYMS		0000010
#define  COFF_F_MINMAL		0000020
#define  COFF_F_UPDATE		0000040
#define  COFF_F_SWABD		0000100
#define  COFF_F_AR16WR		0000200
#define  COFF_F_AR32WR		0000400
#define  COFF_F_AR32W		0001000
#define  COFF_F_PATCH		0002000
#define  COFF_F_NODF		0002000
/* T.Koba */
#define  COFF_F_SYMMERGE	0010000

/* T.Koba */
#define COFF_TGTID_C55X		0x009c

#if 0   /* Perhaps, someday, these formats may be used.      */
#define COFF_I386PTXMAGIC	0x154
#define COFF_I386AIXMAGIC	0x175   /* IBM's AIX system  */
#define COFF_I386BADMAG(x) ((COFF_SHORT((x).f_magic) != COFF_I386MAGIC) \
			  && COFF_SHORT((x).f_magic) != COFF_I386PTXMAGIC \
			  && COFF_SHORT((x).f_magic) != COFF_I386AIXMAGIC)
#else
#define COFF_I386BADMAG(x) (COFF_SHORT((x).f_magic) != COFF_I386MAGIC)
#endif

#define	COFF_FILHDR	struct COFF_filehdr
/*
 * T.Koba:
 * arm-linux-gcc is confused with the struct size of not multiple of 4.
 * It doesn't recognize #pragma pack(1).
 */
//#define	COFF_FILHSZ	sizeof(COFF_FILHDR)
#define	COFF_FILHSZ	22

/********************** AOUT "OPTIONAL HEADER" **********************/

/* Linux COFF must have this "optional" header. Standard COFF has no entry
   location for the "entry" point. They normally would start with the first
   location of the .text section. This is not a good idea for linux. So,
   the use of this "optional" header is not optional. It is required.

   Do not be tempted to assume that the size of the optional header is
   a constant and simply index the next byte by the size of this structure.
   Use the 'f_opthdr' field in the main coff header for the size of the
   structure actually written to the file!!
*/

typedef struct 
{
  char 	magic[2];		/* type of file				 */
  char	vstamp[2];		/* version stamp			 */
  char	tsize[4];		/* text size in bytes, padded to FW bdry */
  char	dsize[4];		/* initialized   data "   "		 */
  char	bsize[4];		/* uninitialized data "   "		 */
  char	entry[4];		/* entry pt.				 */
  char 	text_start[4];		/* base of text used for this file       */
  char 	data_start[4];		/* base of data used for this file       */
}
COFF_AOUTHDR;

#define COFF_AOUTSZ (sizeof(COFF_AOUTHDR))

#define COFF_STMAGIC	0401
#define COFF_OMAGIC     0404
#define COFF_JMAGIC     0407    /* dirty text and data image, can't share  */
#define COFF_DMAGIC     0410    /* dirty text segment, data aligned        */
#define COFF_ZMAGIC     0413    /* The proper magic number for executables  */
#define COFF_SHMAGIC	0443	/* shared library header                   */

/********************** SECTION HEADER **********************/

struct COFF_scnhdr {
  char		s_name[8];	/* section name			    */
  char		s_paddr[4];	/* physical address, aliased s_nlib */
  char		s_vaddr[4];	/* virtual address		    */
  char		s_size[4];	/* section size			    */
  char		s_scnptr[4];	/* file ptr to raw data for section */
  char		s_relptr[4];	/* file ptr to relocation	    */
  char		s_lnnoptr[4];	/* file ptr to line numbers	    */
  char		s_nreloc[4];	/* number of relocation entries	    */
  char		s_nlnno[4];	/* number of line number entries    */
  char		s_flags[4];	/* flags			    */
  char		s_rsvd[2];	/* reserved			    */
  char		s_mempg[2];	/* memory page number	    */
};

#define	COFF_SCNHDR	struct COFF_scnhdr
#define	COFF_SCNHSZ	sizeof(COFF_SCNHDR)

/*
 * names of "special" sections
 */

#define COFF_TEXT	".text"
#define COFF_DATA	".data"
#define COFF_BSS	".bss"
#define COFF_COMMENT    ".comment"
#define COFF_LIB        ".lib"

#define COFF_SECT_TEXT  0      /* Section for instruction code             */
#define COFF_SECT_DATA  1      /* Section for initialized globals          */
#define COFF_SECT_BSS   2      /* Section for un-initialized globals       */
#define COFF_SECT_REQD  3      /* Minimum number of sections for good file */

#define COFF_STYP_REG     0x00 /* regular segment                          */
#define COFF_STYP_DSECT   0x01 /* dummy segment                            */
#define COFF_STYP_NOLOAD  0x02 /* no-load segment                          */
#define COFF_STYP_GROUP   0x04 /* group segment                            */
#define COFF_STYP_PAD     0x08 /* .pad segment                             */
#define COFF_STYP_COPY    0x10 /* copy section                             */
#define COFF_STYP_TEXT    0x20 /* .text segment                            */
#define COFF_STYP_DATA    0x40 /* .data segment                            */
#define COFF_STYP_BSS     0x80 /* .bss segment                             */
#define COFF_STYP_RDATA  0x100 /* read-only (T.Koba)                       */
#define COFF_STYP_INFO   0x200 /* .comment section                         */
#define COFF_STYP_OVER   0x400 /* overlay section                          */
#define COFF_STYP_LIB    0x800 /* library section                          */
#define COFF_STYP_CLINK 0x4000 /* conditionally linked section (T.Koba)    */

/*
 * Shared libraries have the following section header in the data field for
 * each library.
 */

struct COFF_slib {
  char		sl_entsz[4];	/* Size of this entry               */
  char		sl_pathndx[4];	/* size of the header field         */
};

#define	COFF_SLIBHD	struct COFF_slib
#define	COFF_SLIBSZ	sizeof(COFF_SLIBHD)

/********************** LINE NUMBERS **********************/

/* 1 line number entry for every "breakpointable" source line in a section.
 * Line numbers are grouped on a per function basis; first entry in a function
 * grouping will have l_lnno = 0 and in place of physical address will be the
 * symbol table index of the function name.
 */

struct COFF_lineno {
  union {
    char l_symndx[4];	/* function name symbol index, iff l_lnno == 0*/
    char l_paddr[4];	/* (physical) address of line number	*/
  } l_addr;
  char l_lnno[2];	/* line number		*/
};

#define	COFF_LINENO	struct COFF_lineno
#define	COFF_LINESZ	6

/*
 * cinit structure
 * T.Koba
 */
struct COFF_cinit {
  char size[2];
  char addr[3];
  char flags[1];
  char data[0];
};

#define	COFF_CINIT	struct COFF_cinit
#define	COFF_CINITSZ	6

#define COFF_CINIT_FLAG_MEMORY	0x00
#define COFF_CINIT_FLAG_IO	0x01

/********************** SYMBOLS **********************/

#define COFF_E_SYMNMLEN	 8	/* # characters in a short symbol name	*/
#define COFF_E_FILNMLEN	14	/* # characters in a file name		*/
#define COFF_E_DIMNUM	 4	/* # array dimensions in auxiliary entry */

/*
 *  All symbols and sections have the following definition
 */

struct COFF_syment 
{
  union {
    char e_name[E_SYMNMLEN];    /* Symbol name (first 8 characters) */
    struct {
      char e_zeroes[4];         /* Leading zeros */
      char e_offset[4];         /* Offset if this is a header section */
    } e;
  } e;

  char e_value[4];              /* Value (address) of the segment */
  char e_scnum[2];              /* Section number */
  char e_type[2];               /* Type of section */
  char e_sclass[1];             /* Loader class */
  char e_numaux[1];             /* Number of auxiliary entries which follow */
};

#define COFF_N_BTMASK	(0xf)   /* Mask for important class bits */
#define COFF_N_TMASK	(0x30)  /* Mask for important type bits  */
#define COFF_N_BTSHFT	(4)     /* # bits to shift class field   */
#define COFF_N_TSHIFT	(2)     /* # bits to shift type field    */

/*
 *  Storage class for c55x (T.Koba)
 */
#define COFF_C_NULL	0
#define COFF_C_AUTO	1
#define COFF_C_EXT	2
#define COFF_C_STAT	3
#define COFF_C_REG	4
#define COFF_C_EXTREF	5
#define COFF_C_LABEL	6
#define COFF_C_ULABEL	7
#define COFF_C_MOS	8
#define COFF_C_ARG	9
#define COFF_C_STRTAG	10
#define COFF_C_MOU	11
#define COFF_C_UNTAG	12
#define COFF_C_TPDEF	13
#define COFF_C_USTATIC	14
#define COFF_C_ENTAG	15
#define COFF_C_MOE	16
#define COFF_C_REGRARM	17
#define COFF_C_FIELD	18
#define COFF_C_BLOCK	100
#define COFF_C_FCN	101
#define COFF_C_EOS	102
#define COFF_C_FILE	103
#define COFF_C_LINE	104

/*
 *  Symbol type for c55x (T.Koba)
 */
#define COFF_T_NULL	0
#define COFF_T_CHAR	2
#define COFF_T_SHORT	3
#define COFF_T_INT	4
#define COFF_T_LONG	5
#define COFF_T_FLOAT	6
#define COFF_T_DOUBLE	7
#define COFF_T_STRUCT	8
#define COFF_T_UNION	9
#define COFF_T_ENUM	10
#define COFF_T_MOE	11
#define COFF_T_UCHAR	12
#define COFF_T_USHORT	13

#define COFF_DT_NON	0
#define COFF_DT_PTR	1
#define COFF_DT_FCN	2
#define COFF_DT_ARY	3

/*
 *  Auxiliary entries because the main table is too limiting.
 */
  
union COFF_auxent {

/*
 *  Debugger information
 */

  struct {
    char x_tagndx[4];	        /* str, un, or enum tag indx */
    union {
      struct {
	char  x_lnno[2];        /* declaration line number */
	char  x_size[2];        /* str/union/array size */
      } x_lnsz;
      char x_fsize[4];	        /* size of function */
    } x_misc;

    union {
      struct {		        /* if ISFCN, tag, or .bb */
	char x_lnnoptr[4];	/* ptr to fcn line # */
	char x_endndx[4];	/* entry ndx past block end */
      } x_fcn;

      struct {		        /* if ISARY, up to 4 dimen. */
	char x_dimen[E_DIMNUM][2];
      } x_ary;
    } x_fcnary;

    char x_tvndx[2];	/* tv index */
  } x_sym;

/*
 *   Source file names (debugger information)
 */

  union {
    char x_fname[E_FILNMLEN];
    struct {
      char x_zeroes[4];
      char x_offset[4];
    } x_n;
  } x_file;

/*
 *   Section information
 */

  struct {
    char x_scnlen[4];	/* section length */
    char x_nreloc[2];	/* # relocation entries */
    char x_nlinno[2];	/* # line numbers */
  } x_scn;

/*
 *   Transfer vector (branch table)
 */
  
  struct {
    char x_tvfill[4];	/* tv fill value */
    char x_tvlen[2];	/* length of .tv */
    char x_tvran[2][2];	/* tv range */
  } x_tv;		/* info about .tv section (in auxent of symbol .tv)) */
};

#define	COFF_SYMENT	struct COFF_syment
#define	COFF_SYMESZ	18	
#define	COFF_AUXENT	union COFF_auxent
#define	COFF_AUXESZ	18

#define COFF_ETEXT	"etext"

/********************** RELOCATION DIRECTIVES **********************/

struct COFF_reloc {
  char r_vaddr[4];        /* Virtual address of item    */
  char r_symndx[4];       /* Symbol index in the symtab */
  char r_exa[2];          /* extended address calc      */
  char r_type[2];         /* Relocation type            */
};

/*
 * Relocation Type: T.Koba
 */
#define COFF_R_ABS		0x0000
#define COFF_R_REL24		0x0005
#define COFF_R_RELBYTE		0x000f
#define COFF_R_RELWORD		0x0010
#define COFF_R_RELLONG		0x0011
#define COFF_R_LD3_DMA		0x0078
#define COFF_R_LD3_MDP		0x007a
#define COFF_R_LD3_PDP		0x007b
#define COFF_R_LD3_REL23	0x007c
#define COFF_R_LD3_k8		0x0088
#define COFF_R_LD3_k16		0x0089
#define COFF_R_LD3_K8		0x008a
#define COFF_R_LD3_K16		0x008b
#define COFF_R_LD3_l8		0x008c
#define COFF_R_LD3_l16		0x008d
#define COFF_R_LD3_L8		0x008e
#define COFF_R_LD3_L16		0x008f
#define COFF_R_LD3_k4		0x0090
#define COFF_R_LD3_k5		0x0091
#define COFF_R_LD3_K5		0x0092
#define COFF_R_LD3_k6		0x0093
#define COFF_R_LD3_k12		0x0094
#define COFF_R_LD3_REL16	0x0095 // hacked

#define COFF_R_EXTRA_BIT	0x4000
#define COFF_R_EX_ADD		0x4000 // hacked
#define COFF_R_EX_SUB		0x4001 // hacked
#define COFF_R_EX_RSHIFT	0x4007 // hacked
#define COFF_R_EX_MASK		0x4009 // hacked
#define COFF_R_EX_WRITE1	0x400f // hacked (???)
#define COFF_R_EX_WRITE2	0x4010 // hacked (???)
#define COFF_R_EX_PSHSYM	0x4011 // hacked
#define COFF_R_EX_PSHVAL	0x4013 // hacked
#define COFF_R_EX_PSHINTOFF	0x4014 // hacked
#define COFF_R_EX_WRITE3	0x4016 // hacked (???)

#define COFF_RELOC struct COFF_reloc
#define COFF_RELSZ 12

#define COFF_DEF_DATA_SECTION_ALIGNMENT  4
#define COFF_DEF_BSS_SECTION_ALIGNMENT   4
#define COFF_DEF_TEXT_SECTION_ALIGNMENT  4

/* For new sections we haven't heard of before */
#define COFF_DEF_SECTION_ALIGNMENT       4

#endif /* __COFF_C55X_H */
