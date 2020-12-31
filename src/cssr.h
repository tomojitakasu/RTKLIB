/*------------------------------------------------------------------------------
* cssr.h : Compact SSR constants, types and function prototypes
*
*          Copyright (C) 2015- by Mitsubishi Electric Corporation, All rights reserved.
*-----------------------------------------------------------------------------*/
#ifndef CSSR_H
#define CSSR_H
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <ctype.h>
#if !defined(__CYGWIN__)
#include <stdint.h>
#endif

/* constants -----------------------------------------------------------------*/

#define VER_CSSR  "0.9"             /* library version */

/* cssr */
#define CSSR_MAX_GNSS     16
#define CSSR_MAX_SV_GNSS  40
#define CSSR_MAX_SV       64
#define CSSR_MAX_SIG      16
#define CSSR_MAX_CELLMASK 64
#define CSSR_MAX_NET      32
#define CSSR_MAX_LOCAL_SV 32
#define CSSR_MAX_GP       128
#define CSSR_MAX_NETWORK  32

#define CSSR_SYS_GPS    0
#define CSSR_SYS_GLO    1
#define CSSR_SYS_GAL    2
#define CSSR_SYS_BDS    3
#define CSSR_SYS_QZS    4
#define CSSR_SYS_SBS    5
#define CSSR_SYS_IRN    6
#define CSSR_SYS_BDS3   7
#define CSSR_SYS_NONE   -1

#define CSSR_TYPE_NUM   14

#define CSSR_TYPE_MASK  1
#define CSSR_TYPE_OC    2
#define CSSR_TYPE_CC    3
#define CSSR_TYPE_CB    4
#define CSSR_TYPE_PB    5
#define CSSR_TYPE_BIAS  6
#define CSSR_TYPE_URA   7
#define CSSR_TYPE_STEC  8
#define CSSR_TYPE_GRID  9
#define CSSR_TYPE_SI    10
#define CSSR_TYPE_OCC   11
#define CSSR_TYPE_ATMOS 12

#define CSSR_TYPE_TEST 15
#define CSSR_SUBTYPE_AUTH 1

#define CSSR_TYPE_INIT  254
#define CSSR_TYPE_NULL  255

#define CSSR_SI_GRID 	3
#define CSSR_SI_COORD	4

#define P2_S16_MAX 32767
#define P2_S15_MAX 16383
#define P2_S14_MAX 8191
#define P2_S13_MAX 4095
#define P2_S12_MAX 2047
#define P2_S11_MAX 1023
#define P2_S10_MAX 511
#define P2_S9_MAX  255
#define P2_S8_MAX  127
#define P2_S7_MAX  63
#define P2_S6_MAX  31

#define CSSR_TROP_HS_REF    2.3
#define CSSR_TROP_WET_REF   0.252

#define CSSR_UPDATE_TROP	0
#define CSSR_UPDATE_STEC	1
#define CSSR_UPDATE_PBIAS	2

#define INVALID_VALUE -10000

#define CSSR_BIAS_CBIAS 1
#define CSSR_BIAS_PBIAS 2

#define CSSR_CTYPE_MASK		1
#define CSSR_CTYPE_OC		2
#define CSSR_CTYPE_CC		4
#define CSSR_CTYPE_BIAS		8
#define CSSR_CTYPE_STEC		16
#define CSSR_CTYPE_GRID		32
#define CSSR_CTYPE_ATM		64
#define CSSR_CTYPE_OCC		128

#define CSSR_AUTH_OTAR_MAX 64

typedef struct {
	int msgno;
	float udint[16];
    uint8_t stec_type;
    uint8_t trop_type;
    uint8_t ocb_avail[3];
    uint8_t netmode[3];
    uint8_t atmos_mode;
    uint8_t stec_ctype[CSSR_MAX_LOCAL_SV];
    double sig_dstec;
    uint8_t stec_size[CSSR_MAX_LOCAL_SV];
    uint8_t trop_size;
    uint8_t trop_ctype;
    uint8_t stec_range;
    uint8_t trop_avail;
    uint8_t stec_avail;
    uint8_t auth_type;
    uint8_t crypt_type;
    uint8_t num_msg;
    uint8_t maclen;
    uint8_t keylen;
} cssropt_t;

typedef struct {
   /* gtime_t t0[2];*/
    double udi[2];
    int iod[2];
    int ngp;
    float quality_f[CSSR_MAX_LOCAL_SV];
    float trop_wet[CSSR_MAX_GP];
    float trop_total[CSSR_MAX_GP];
    int nsat_f;
    int sat_f[CSSR_MAX_LOCAL_SV];
    float quality;
    double a[CSSR_MAX_LOCAL_SV][4];
    int nsat[CSSR_MAX_GP];
    int sat[CSSR_MAX_GP][CSSR_MAX_LOCAL_SV];
    float stec[CSSR_MAX_GP][CSSR_MAX_LOCAL_SV];
    double grid[CSSR_MAX_GP][3];
    int update[3];
    double ci[CSSR_MAX_LOCAL_SV][5];
    double ct[5];
    uint8_t i_excl[CSSR_MAX_LOCAL_SV];
    double trop_offset;
} ssrn_t;

typedef struct {
	uint8_t aid;
	uint8_t fpart;
	uint8_t npart;
	uint8_t pid;
	uint8_t type;
	uint8_t ngrid;
	double lat0;
	double lon0;
	float slat;
	float slon;
	float dlat[CSSR_MAX_GP];
	float dlon[CSSR_MAX_GP];
	uint8_t ncount[2];
	uint8_t fmask;
	uint64_t mask;
	uint8_t size[CSSR_MAX_NETWORK];
} cssr_si_grid_t;

typedef struct {
	uint8_t iod;
	uint8_t narea;
	cssr_si_grid_t grd[CSSR_MAX_NETWORK];
	uint8_t np[CSSR_MAX_NETWORK];
} cssr_si_area_t;

typedef struct {
	uint8_t aid;
	uint8_t avail_f[3];
	uint8_t avail_g[3];
	uint8_t f_type[3];
	uint8_t grid_type[CSSR_MAX_NETWORK];
	uint8_t aid_custom[CSSR_MAX_NETWORK];
	uint8_t ngrid[CSSR_MAX_NETWORK];
	uint8_t sz_grid[CSSR_MAX_NETWORK];
	double c[CSSR_MAX_NETWORK][5];
	double d[CSSR_MAX_NETWORK][CSSR_MAX_GP];
} cssr_si_trans_t;


typedef struct {
	uint8_t type;
	uint8_t iod;
	uint8_t src_id;
	uint8_t dst_id;
	uint8_t avail_hv;
	uint16_t ref_week;
	uint8_t narea;
	cssr_si_trans_t f[CSSR_MAX_NETWORK];
} cssr_si_ct_t;

typedef struct {
	uint8_t type;
	uint8_t iod;
	cssr_si_area_t area;
	cssr_si_ct_t ct;
	int nbit;
	uint8_t buff[512];
} cssr_si_t;

typedef struct {
	uint8_t num_msg;
	uint8_t hmac[10];
	uint8_t key_hmac[128];
	uint8_t otar_counter;
	uint8_t otar[CSSR_AUTH_OTAR_MAX];
} cssr_auth_t;

typedef struct {
    int ver;
    cssropt_t opt;
    int nbit;
    int iod;
    int iod_sv;
    int inet;
    int week;
    double tow0;
   /* gtime_t time_ref;*/
    uint8_t cmi[CSSR_MAX_GNSS]; /* cellmask existence flag */
    uint64_t svmask[CSSR_MAX_GNSS];
    uint16_t sigmask[CSSR_MAX_GNSS];
    uint16_t cellmask[CSSR_MAX_SV];
    uint64_t net_svmask[CSSR_MAX_NET];
    int ngnss;
    int nsat;
    int ncell;
    int sat[CSSR_MAX_SV];
    int nsat_n[CSSR_MAX_NET];
    int sat_n[CSSR_MAX_NET][CSSR_MAX_LOCAL_SV];
    int nsig[CSSR_MAX_SV];
    int sigmask_s[CSSR_MAX_SV];
#if 0
    int amb_bias[MAXSAT][MAXCODE];
    uint8_t disc[MAXSAT][MAXCODE];
#endif
    float quality_i;    /* ionosphere quality */
    int l6delivery;
    int l6facility;
    int si_cnt;
    int si_sz;
    uint8_t flg_cssr_si;
    uint64_t si_data[4];
	ssrn_t ssrn[CSSR_MAX_NET];
	uint8_t idx[CSSR_MAX_GNSS][CSSR_MAX_LOCAL_SV];
	cssr_si_t si;
	cssr_auth_t auth;
} cssr_t;

#define CSSR_OTYPE_RTCM3	1
#define CSSR_OTYPE_L6		2

#define L6MSG_LENGTH 	256
#define LEN_L6MSG_DATA	1695

typedef struct {
	uint8_t alert;
	uint8_t prn;
	uint8_t vendor_id;
	uint8_t facility_id;
	uint8_t subframe_length;
	int nbit;
	uint8_t buff[L6MSG_LENGTH];
} l6msg_t;

#endif /* CSSR_H */
