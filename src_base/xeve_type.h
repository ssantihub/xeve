/* Copyright (c) 2020, Samsung Electronics Co., Ltd.
   All Rights Reserved. */
/*
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:
   
   - Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.
   
   - Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.
   
   - Neither the name of the copyright owner, nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.
   
   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED.IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef _XEVE_TYPE_H_
#define _XEVE_TYPE_H_

#include "xeve_def.h"
#include "xeve_bsw.h"
#include "xeve_sad.h"
#include "xeve_sad_sse.h"
#include "xeve_sad_avx.h"

/* support RDOQ */
#define SCALE_BITS               15    /* Inherited from TMuC, pressumably for fractional bit estimates in RDOQ */
#define ERR_SCALE_PRECISION_BITS 20

/* XEVE encoder magic code */
#define XEVE_MAGIC_CODE      0x45565945 /* EVYE */

/* Max. and min. Quantization parameter */
#define MAX_QUANT                51
#define MIN_QUANT                0

/* count of picture including encoding and reference pictures
0: encoding picture buffer
1: forward reference picture buffer
2: backward reference picture buffer, if exists
3: original (input) picture buffer
4: mode decision picture buffer, if exists
*/
#define PIC_D                    5
/* current encoding picture buffer index */
#define PIC_IDX_CURR             0
/* list0 reference picture buffer index */
#define PIC_IDX_FORW             1
/* list1 reference picture buffer index */
#define PIC_IDX_BACK             2
/* original (input) picture buffer index */
#define PIC_IDX_ORIG             3
/* mode decision picture buffer index */
#define PIC_IDX_MODE             4

/* check whether bumping is progress or not */
#define FORCE_OUT(ctx)          (ctx->param.force_output == 1)

/* motion vector accuracy level for inter-mode decision */
#define ME_LEV_IPEL              1
#define ME_LEV_HPEL              2
#define ME_LEV_QPEL              3

/* maximum inbuf count */
#define XEVE_MAX_INBUF_CNT   34

/* maximum cost value */
#define MAX_COST                (1.7e+308)

/*****************************************************************************
 * mode decision structure
 *****************************************************************************/
typedef struct _XEVE_MODE
{
    void *pdata[4];
    int  *ndata[4];
    pel  *rec[N_C];
    int   s_rec[N_C];
    /* CU count in a CU row in a LCU (== log2_max_cuwh - MIN_CU_LOG2) */
    u8    log2_culine;
    /* reference indices */
    s8    refi[REFP_NUM];
    /* MVP indices */
    u8    mvp_idx[REFP_NUM];
    /* MVR indices */
    u8    mvr_idx;
    u8    bi_idx;
    s16   mmvd_idx;
    /* mv difference */
    s16   mvd[REFP_NUM][MV_D];    
    /* mv */
    s16   dmvr_mv[MAX_CU_CNT_IN_LCU][REFP_NUM][MV_D];
    /* mv */
    s16   mv[REFP_NUM][MV_D];
    pel  *pred_y_best;
    s16   affine_mv[REFP_NUM][VER_NUM][MV_D];
    s16   affine_mvd[REFP_NUM][VER_NUM][MV_D];
    int   cu_mode;
    u8    affine_flag;
    // spatial neighboring MV of affine block
    s8    refi_sp[REFP_NUM];
    s16   mv_sp[REFP_NUM][MV_D];
    u8    ats_intra_cu;
    u8    ats_intra_mode_h;
    u8    ats_intra_mode_v;
#if TRACE_ENC_CU_DATA
    u64   trace_cu_idx;
#endif
#if TRACE_ENC_HISTORIC
    XEVE_HISTORY_BUFFER     history_buf;
#endif
} XEVE_MODE;

/* virtual frame depth B picture */
#define FRM_DEPTH_0                   0
#define FRM_DEPTH_1                   1
#define FRM_DEPTH_2                   2
#define FRM_DEPTH_3                   3
#define FRM_DEPTH_4                   4
#define FRM_DEPTH_5                   5
#define FRM_DEPTH_6                   6
#define FRM_DEPTH_MAX                 7
/* I-slice, P-slice, B-slice + depth + 1 (max for GOP 8 size)*/
#define LIST_NUM                      1

/*****************************************************************************
 * pre-defined structure
 *****************************************************************************/
typedef struct _XEVE_CTX XEVE_CTX;
typedef struct _XEVE_ALF XEVE_ALF;
typedef struct _XEVE_CORE XEVE_CORE;
typedef struct _XEVE_IBC_HASH XEVE_IBC_HASH;
typedef struct _XEVE_RC_PARAM XEVE_RC_PARAM;

/*****************************************************************************
 * original picture buffer structure
 *****************************************************************************/
typedef struct _XEVE_PICO
{
    /* original picture store */
    XEVE_PIC            pic;
    /* input picture count */
    u32                 pic_icnt;
    /* be used for encoding input */
    u8                  is_used;
    /* address of sub-picture */
    XEVE_PIC          * spic;
    s32                 slice_type;
    s32                 slice_depth;
    s32                 scene_type;
    /* adaptive quantization qp offset */
    s32               * map_qp_offset;
    /*[0] intra [1]: Uni_1 / [2]: Uni_2 / [3]: Uni_3 */
    s32                 uni_est_cost[4];
    s32                 bi_fcost;
    /* number of intra unit ([0]: ICNT_P1 /[1]: ICNT_P2 / [2]: ICNT_PGA) */
    u16                 icnt[3];
    /* pred direction map (PRED_L0, PRED_L1, PRED_BI) */
    u8                * map_pdir;
    /* lcu-tree transfer cost */
    u16               * transfer_cost;
    /* sub-picture motion vector map for every 32x32 unit */
    s16              (* map_mv)[REFP_NUM][MV_D];
    s16              (* map_mv_pga)[REFP_NUM][MV_D];
    /* uni direction lcu cost
    [0] : intra lcu cost
    [1] : inter lcu cost with -1 picture
    [2] : inter lcu cost with -2 picture
    [3] : inter lcu cost with the previous gop anchor */
    s32              (* map_lcu_cost_uni)[4];
    /* bi-inter lcu cost */
    s32               * map_lcu_cost_bi;
} XEVE_PICO;

/*****************************************************************************
 * intra prediction structure
 *****************************************************************************/
typedef struct _XEVE_PINTRA
{
    /* temporary prediction buffer */
    pel                 pred[N_C][MAX_CU_DIM];
    pel                 pred_cache[IPD_CNT][MAX_CU_DIM]; // only for luma

    /* reconstruction buffer */
    pel                 rec[N_C][MAX_CU_DIM];

    s16                 coef_tmp[N_C][MAX_CU_DIM];
    s16                 coef_best[N_C][MAX_CU_DIM];
    int                 nnz_best[N_C];
    int                 nnz_sub_best[N_C][MAX_SUB_TB_NUM];
    pel                 rec_best[N_C][MAX_CU_DIM];

    /* original (input) picture buffer */
    XEVE_PIC          * pic_o;
    /* address of original (input) picture buffer */
    pel               * o[N_C];
    /* stride of original (input) picture buffer */
    int                 s_o[N_C];
    /* mode picture buffer */
    XEVE_PIC          * pic_m;
    /* address of mode picture buffer */

    pel               * m[N_C];
    /* stride of mode picture buffer */
    int                 s_m[N_C];

    /* QP for luma */
    u8                  qp_y;
    /* QP for chroma */
    u8                  qp_u;
    u8                  qp_v;

    int                 slice_type;

    int                 complexity;
    void              * pdata[4];
    int               * ndata[4];
} XEVE_PINTRA;

/*****************************************************************************
 * inter prediction structure
 *****************************************************************************/
#define MV_RANGE_MIN           0
#define MV_RANGE_MAX           1
#define MV_RANGE_DIM           2

typedef struct _XEVE_PRED_INTER_COMP
{
    u8 raster_search_step_opt;
    u8 search_step_max;
    u8 search_step_min;
    u8 raster_new_center_th;
    u8 max_first_search_step_th;
    u8 max_refine_search_step_th;
    u8 opt_me_diamond_mvr012_step;
    u8 mvr_012_bi_step;
    u8 mvr_012_non_bi_step;
    u8 bi_normal_step_c;
    u8 bi_normal_mask;
    u8 mvr_02_step_nxt;
    u8 mvr_012_step_th;

} XEVE_PRED_INTER_COMP;

typedef struct _XEVE_PINTER XEVE_PINTER;
struct _XEVE_PINTER
{
    /* temporary prediction buffer (only used for ME)*/
    pel                 pred_buf[MAX_CU_DIM];
    /* temporary buffer for analyze_cu */
    s8                  refi[PRED_NUM][REFP_NUM];
    /* Ref idx predictor */
    s8                  refi_pred[REFP_NUM][MAX_NUM_MVP]; 
    u8                  mvp_idx[PRED_NUM][REFP_NUM];
    s16                 mvp_scale[REFP_NUM][MAX_NUM_ACTIVE_REF_FRAME][MAX_NUM_MVP][MV_D];
    s16                 mv_scale[REFP_NUM][MAX_NUM_ACTIVE_REF_FRAME][MV_D];
    u8                  mvp_idx_temp_for_bi[PRED_NUM][REFP_NUM][MAX_NUM_ACTIVE_REF_FRAME];
    int                 best_index[PRED_NUM][4];
    s16                 mmvd_idx[PRED_NUM];
    u8                  mvr_idx[PRED_NUM];
    u8                  curr_mvr;
    int                 max_imv[MV_D];
    s8                  first_refi[PRED_NUM][REFP_NUM];
    u8                  bi_idx[PRED_NUM];
    u8                  curr_bi;
    int                 max_search_range;
    s16                 affine_mvp_scale[REFP_NUM][MAX_NUM_ACTIVE_REF_FRAME][MAX_NUM_MVP][VER_NUM][MV_D];
    s16                 affine_mv_scale[REFP_NUM][MAX_NUM_ACTIVE_REF_FRAME][VER_NUM][MV_D];
    u8                  mvp_idx_scale[REFP_NUM][MAX_NUM_ACTIVE_REF_FRAME];
    s16                 affine_mvp[REFP_NUM][MAX_NUM_MVP][VER_NUM][MV_D];
    s16                 affine_mv[PRED_NUM][REFP_NUM][VER_NUM][MV_D];
    s16                 affine_mvd[PRED_NUM][REFP_NUM][VER_NUM][MV_D];
    pel                 p_error[MAX_CU_DIM];
    int                 i_gradient[2][MAX_CU_DIM];
    s16                 resi[N_C][MAX_CU_DIM];
    s16                 coff_save[N_C][MAX_CU_DIM];
    u8                  ats_inter_info_mode[PRED_NUM];
    /* MV predictor */
    s16                 mvp[REFP_NUM][MAX_NUM_MVP][MV_D]; 
    s16                 dmvr_mv[PRED_NUM][MAX_CU_CNT_IN_LCU][REFP_NUM][MV_D];
    s16                 mv[PRED_NUM][REFP_NUM][MV_D];
    s16                 mvd[PRED_NUM][REFP_NUM][MV_D];
    s16                 org_bi[MAX_CU_DIM];
    s32                 mot_bits[REFP_NUM];
    /* temporary prediction buffer (only used for ME)*/
    pel                 pred[PRED_NUM+1][2][N_C][MAX_CU_DIM];
    pel                 dmvr_template[MAX_CU_DIM];
    pel                 dmvr_half_pred_interpolated[REFP_NUM][(MAX_CU_SIZE + 1) * (MAX_CU_SIZE + 1)];
    pel                 dmvr_padding_buf[PRED_NUM][N_C][PAD_BUFFER_STRIDE * PAD_BUFFER_STRIDE];
    pel                 dmvr_ref_pred_interpolated[REFP_NUM][(MAX_CU_SIZE + ((DMVR_NEW_VERSION_ITER_COUNT + 1) * REF_PRED_EXTENTION_PEL_COUNT)) * (MAX_CU_SIZE + ((DMVR_NEW_VERSION_ITER_COUNT + 1) * REF_PRED_EXTENTION_PEL_COUNT))];
    /* reconstruction buffer */
    pel                 rec[PRED_NUM][N_C][MAX_CU_DIM];
    /* last one buffer used for RDO */
    s16                 coef[PRED_NUM+1][N_C][MAX_CU_DIM];
    s16                 residue[N_C][MAX_CU_DIM];
    int                 nnz_best[PRED_NUM][N_C];
    int                 nnz_sub_best[PRED_NUM][N_C][MAX_SUB_TB_NUM];
    u8                  num_refp;
    /* minimum clip value */
    s16                 min_clip[MV_D];
    /* maximum clip value */
    s16                 max_clip[MV_D];
    /* search range for int-pel */
    s16                 search_range_ipel[MV_D];
    /* search range for sub-pel */
    s16                 search_range_spel[MV_D];
    s8              ( * search_pattern_hpel)[2];
    u8                  search_pattern_hpel_cnt;
    s8              ( * search_pattern_qpel)[2];
    u8                  search_pattern_qpel_cnt;
    /* original (input) picture buffer */
    XEVE_PIC          * pic_o;
    /* address of original (input) picture buffer */
    pel               * o[N_C];
    /* stride of original (input) picture buffer */
    int                 s_o[N_C];
    /* mode picture buffer */
    XEVE_PIC          * pic_m;
    /* address of mode picture buffer */
    pel               * m[N_C];
    /* stride of mode picture buffer */
    int                 s_m[N_C];
    /* motion vector map */
    s16             ( * map_mv)[REFP_NUM][MV_D];
    /* unrefined motion vector map */
    s16             ( * map_unrefined_mv)[REFP_NUM][MV_D];
    /* picture width in SCU unit */
    u16                 w_scu;
    /* QP for luma of current encoding CU */
    u8                  qp_y;
    /* QP for chroma of current encoding CU */
    u8                  qp_u;
    u8                  qp_v;
    u32                 lambda_mv;
    /* reference pictures */
    XEVE_REFP       ( * refp)[REFP_NUM];
    int                 slice_type;
    /* search level for motion estimation */
    int                 me_level;
    int                 complexity;
    void              * pdata[4];
    int               * ndata[4];
    /* current picture order count */
    int                 poc;
    /* gop size */
    int                 gop_size;
    int                 sps_amvr_flag;
    int                 skip_merge_cand_num;
    int                 me_complexity;
    s64                 best_ssd;
    XEVE_PRED_INTER_COMP * me_opt;
    /* ME function (Full-ME or Fast-ME) */
    u32 (*fn_me)(XEVE_PINTER *pi, int x, int y, int log2_cuw, int log2_cuh, s8 *refi, int lidx, s16 mvp[MV_D], s16 mv[MV_D], int bi, int bit_depth_luma);
    /* AFFINE ME function (Gradient-ME) */
    u32 (*fn_affine_me)(XEVE_PINTER *pi, int x, int y, int log2_cuw, int log2_cuh, s8 *refi, int lidx, s16 mvp[VER_NUM][MV_D], s16 mv[VER_NUM][MV_D], int bi, int vertex_num, pel *tmp, int bit_depth_luma, int bit_depth_chroma);
    s8 (*fn_get_first_refi)(XEVE_CTX *ctx, XEVE_CORE *core, int ref_idx, int pidx, int cuw, int cuh);
    void (*fn_save_best_info)(XEVE_CTX *ctx, XEVE_CORE *core, int pidx);
    void (*fn_load_best_info)(XEVE_CTX *ctx, XEVE_CORE *core, int pidx);
    void (*fn_mc)(XEVE_CTX *ctx, XEVE_CORE *core, int x, int y, int w, int h, s8 refi[REFP_NUM], s16(*mv)[MV_D], XEVE_REFP(*refp)[REFP_NUM]
                , pel pred[REFP_NUM][N_C][MAX_CU_DIM], int poc_c, int apply_dmvr, s16 dmvr_mv[MAX_CU_CNT_IN_LCU][REFP_NUM][MV_D]);
};

typedef struct _XEVE_PIBC
{
    /* filtered reconstruction buffer */
    pel                 unfiltered_rec_buf[N_C][MAX_CU_DIM];
    /* temporary buffer for analyze_cu */
    s8                  refi[REFP_NUM];
    /* Ref idx predictor */
    s8                  refi_pred[REFP_NUM];
    u8                  pred_mode;
    u8                  ibc_flag;
    int                 search_range_x;
    int                 search_range_y;
    u8                  mvp_idx;
    /* MV predictor */
    s16                 mvp[MAX_NUM_MVP][MV_D];
    s16                 mv[REFP_NUM][MV_D];
    s16                 mvd[MV_D];
    s32                 mot_bits;
    /* last one buffer used for RDO */
    s16                 coef[N_C][MAX_CU_DIM];
    s16                 inv_coef[N_C][MAX_CU_DIM];
    s16                 residue[N_C][MAX_CU_DIM];
    int                 nnz_best[N_C];
    int                 nnz_sub_best[PRED_NUM][N_C][MAX_SUB_TB_NUM];
    /* minimum clip value */
    s16                 min_clip[MV_D];
    /* maximum clip value */
    s16                 max_clip[MV_D];
    /* original (input) picture buffer */
    XEVE_PIC          * pic_o;
    /* address of original (input) picture buffer */
    pel               * o[N_C];
    /* stride of original (input) picture buffer */
    int                 s_o[N_C];
    /* mode picture buffer */
    XEVE_PIC          * pic_m;
    /* address of mode picture buffer */
    pel               * m[N_C];
    /* stride of mode picture buffer */
    int                 s_m[N_C];
    /* ctu size log2 table */
    s8                  ctu_log2_tbl[MAX_CU_SIZE + 1];
    /* temporary prediction buffer (only used for ME)*/
    pel                 pred[REFP_NUM][N_C][MAX_CU_DIM];
    /* picture width in SCU unit */
    u16                 w_scu;
    /* QP for luma of current encoding CU */
    u8                  qp_y;
    /* QP for chroma of current encoding CU */
    u8                  qp_u;
    u8                  qp_v;
    u32                 lambda_mv;
    int                 slice_type;
    int                 complexity;
    void              * pdata[4];
    int               * ndata[4];
} XEVE_PIBC;

/* XEVE parameter */
typedef struct _XEVE_PARAM
{
    /* picture size of input sequence (width) */
    int                 w;
    /* picture size of input sequence (height) */
    int                 h;
    /* picture bit depth*/
    int                 bit_depth;
    /* qp value for I- and P- slice */
    int                 qp;
    /* frame per second */
    int                 fps;
    /* Enable deblocking filter or not
       - 0: Disable deblocking filter
       - 1: Enable deblocking filter
    */
    int                 use_deblock;
    int                 deblock_alpha_offset;
    int                 deblock_beta_offset;
    /* I-frame period */
    int                 i_period;
    /* force I-frame */
    int                 f_ifrm;
    /* Maximum qp value */
    int                 qp_max;
    /* Minimum qp value */
    int                 qp_min;
    /* use picture signature embedding */
    int                 use_pic_sign;
    int                 max_b_frames;
    int                 ref_pic_gap_length;
    /* start bumping process if force_output is on */
    int                 force_output;
    int                 gop_size;
    int                 use_dqp;
    int                 use_closed_gop;
    int                 use_ibc_flag;
    int                 ibc_search_range_x;
    int                 ibc_search_range_y;
    int                 ibc_hash_search_flag;
    int                 ibc_hash_search_max_cand;
    int                 ibc_hash_search_range_4smallblk;
    int                 ibc_fast_method;
    int                 use_hgop;
    /* config parameter for cu_qp_delta_area*/
    int                 cu_qp_delta_area;
    int                 qp_incread_frame;           /* 10 bits*/
    /* number of tile' columns (1-20)*/
    int                 tile_columns;
    /* number of tile' rows (1-22) */
    int                 tile_rows;
    /* flag for uniform spacing tiles */
    int                 uniform_spacing_tiles;
    int                 num_slice_in_pic;
    /* Array for storing slice boundaries*/ 
    /* In XEVE slices are rectangular only (No rasterscan tiles)
    Slice boundaries stores the tile index of top left and bottom right tiles */
    int                 slice_boundary_array[2 * 600];
    int                 tile_array_in_slice[2 * 600];
    int                 arbitrary_slice_flag;
    u32                 num_remaining_tiles_in_slice_minus1[600];
    u8                  rdo_dbk_switch;
    const XEVE_PRESET * preset;
    int                 rc_type;
    int                 bps;
    int                 vbv_msec;
    int                 use_filler_flag;
    int                 num_pre_analysis_frames;
    /* Rate control type (Off CBR ABR) */
    int                 vbv_enabled;
    /* vbv parameters */
    int                 vbv_buffer_size;
    /* vbv buffer size */

} XEVE_PARAM;

/*****************************************************************************
* rate control structure for bits estimating
*****************************************************************************/
#define RC_NUM_SLICE_TYPE  8
typedef struct _XEVE_RCBE
{
    double       bits;
    double       cnt;
    double       coef;
    double       offset;
    double       decayed;
} XEVE_RCBE;

/*****************************************************************************
* rate control structure for encoding
*****************************************************************************/
typedef struct _XEVE_RCORE
{
    u16        * pred;

    /* qf value limitation parameter */
    double       qf_limit;
    /* offset btw I and P frame */
    double       offset_ip;
    /* minimum qfactor by frame type */
    double       qf_min[RC_NUM_SLICE_TYPE];
    /* maximum qfactor by frame type */
    double       qf_max[RC_NUM_SLICE_TYPE];
    /* current frame scene_type which is inherited from frame analysis */
    int          scene_type;
    /* current frame qp */
    double       qp;
    /* complexity for current frame (mad) */
    s32          cpx_frm;
    /* complexity for rc model update */
    double       cpx_pow;
    /* estimated bits (restore for update) */
    double       est_bits;
    /* real bits (restore for update) */
    double       real_bits;
    /* slice type    (restore for update) */
    int          stype;
    /* slice dpeth   (restore for update) */
    int          sdepth;
    int          avg_dqp;
    /* use filler for write extra byte */
    int          filler_byte;
} XEVE_RCORE;

/*****************************************************************************
* rate control structure
*****************************************************************************/
typedef struct _XEVE_RC
{
    /* frame per second */
    double       fps;
    /* bit per second */
    double       bitrate;
    /* allocated bits per frame (bitrate/fps)*/
    double       bpf;
    /* maximum bit size for one frame encoding */
    double       max_frm_bits;
    /* vbv enabled flag */
    int          vbv_enabled;
    /* total vbv buffer size (bitrate * vbv_msec /1000) (constant) */
    double       vbv_buf_size;
    double       lambda[4];

    /* sum of k_param (bits*qfactor/rc_avg_cpx) */
    double       k_param;
    /* accumulated target bitrate * window */
    double       target_bits;
    /* accumulated frame size for each slice type */
    s64          frame_bits;
    /* sum of qp to get I frame qfactor */
    double       qp_sum;
    /* count of qp to get I frame qfactor */
    double       qp_cnt;
    /* sum of complexity */
    double       cpx_sum;
    /* count of complexity */
    double       cpx_cnt;
    /* bpf decayed weight factor */
    double       bpf_decayed;
    /* Rate Control Bits Predictor structure */
    XEVE_RCBE    bit_estimator[RC_NUM_SLICE_TYPE];
    /* amount of vbv buffer fullness */
    double       vbv_buf_fullness;
    /* store slice type of last and previous of last picture I, P slice type
    0 : last picture
    1 : previous of last picture                                           */
    int          prev_st[2];
    /* store qf of last and previous of last picture forI, P slice type
    0 : last picture
    1 : previous of last picture                                           */
    double       prev_qf[2][RC_NUM_SLICE_TYPE];
    /* store poc of last and previous of last picture for I, P slice type
    0 : last picture
    1 : previous of last picture                                           */
    int          prev_picnt[2][RC_NUM_SLICE_TYPE];

    XEVE_RC_PARAM * param;
} XEVE_RC;

typedef struct _XEVE_SBAC
{
    u32                 range;
    u32                 code;
    u32                 code_bits;
    u32                 stacked_ff;
    u32                 stacked_zero;
    u32                 pending_byte;
    u32                 is_pending_byte;
    XEVE_SBAC_CTX       ctx;
    u32                 bitcounter;
    u8                  is_bitcount;
    u32                 bin_counter;
} XEVE_SBAC;
typedef struct _XEVE_DQP
{
    s8                  prev_qp;
    s8                  curr_qp;
    s8                  cu_qp_delta_is_coded;
    s8                  cu_qp_delta_code;
} XEVE_DQP;
typedef struct _XEVE_CU_DATA
{
    s8                 split_mode[NUM_CU_DEPTH][NUM_BLOCK_SHAPE][MAX_CU_CNT_IN_LCU];
    s8                 suco_flag[NUM_CU_DEPTH][NUM_BLOCK_SHAPE][MAX_CU_CNT_IN_LCU];
    u8               * qp_y;
    u8               * qp_u;
    u8               * qp_v;
    u8               * pred_mode;
    u8               * pred_mode_chroma;
    u8              ** mpm;
    u8              ** mpm_ext;
    s8              ** ipm;
    u8               * skip_flag;
    u8               * ibc_flag;
    u8               * dmvr_flag;
    s8              ** refi;    
    u8              ** mvp_idx; 
    u8               * mvr_idx;
    u8               * bi_idx;
    s16              * mmvd_idx;
    u8               * mmvd_flag;
    s16                bv_chroma[MAX_CU_CNT_IN_LCU][MV_D];
    s16                mv[MAX_CU_CNT_IN_LCU][REFP_NUM][MV_D];
    s16                unrefined_mv[MAX_CU_CNT_IN_LCU][REFP_NUM][MV_D];
    s16                mvd[MAX_CU_CNT_IN_LCU][REFP_NUM][MV_D];
    int              * nnz[N_C];
    int              * nnz_sub[N_C][4];
    u32              * map_scu;
    u8               * affine_flag;
    u32              * map_affine;
    u8               * ats_intra_cu;
    u8               * ats_mode_v;
    u8               * ats_mode_h;
    u8               * ats_inter_info;
    u32              * map_cu_mode;
    s8               * depth;
    s16              * coef[N_C]; 
    pel              * reco[N_C]; 
#if TRACE_ENC_CU_DATA
    u64                trace_idx[MAX_CU_CNT_IN_LCU];
#endif
#if TRACE_ENC_HISTORIC
    XEVE_HISTORY_BUFFER     history_buf[MAX_CU_CNT_IN_LCU];
#endif
} XEVE_CU_DATA;

/*****************************************************************************
 * CORE information used for encoding process.
 *
 * The variables in this structure are very often used in encoding process.
 *****************************************************************************/
struct _XEVE_CORE
{
    /* coefficient buffer of current CU */
    s16                coef[N_C][MAX_CU_DIM];
    /* CU data for RDO */
    XEVE_CU_DATA       cu_data_best[MAX_CU_LOG2][MAX_CU_LOG2];
    XEVE_CU_DATA       cu_data_temp[MAX_CU_LOG2][MAX_CU_LOG2];
    XEVE_DQP           dqp_data[MAX_CU_LOG2][MAX_CU_LOG2];
    /* temporary coefficient buffer */
    s16                ctmp[N_C][MAX_CU_DIM];
    /* pred buffer of current CU. [1][x][x] is used for bi-pred */
    pel                pred[2][N_C][MAX_CU_DIM];
    /* neighbor pixel buffer for intra prediction */
    pel                nb[N_C][N_REF][MAX_CU_SIZE * 3];
    /* current encoding LCU number */
    int                lcu_num;
    /*QP for current encoding CU. Used to derive Luma and chroma qp*/
    u8                 qp;
    u8                 cu_qp_delta_code;
    u8                 cu_qp_delta_is_coded;
    u8                 cu_qp_delta_code_mode;
    XEVE_DQP           dqp_curr_best[MAX_CU_LOG2][MAX_CU_LOG2];
    XEVE_DQP           dqp_next_best[MAX_CU_LOG2][MAX_CU_LOG2];
    XEVE_DQP           dqp_temp_best;
    XEVE_DQP           dqp_temp_best_merge;
    XEVE_DQP           dqp_temp_run;
    /* QP for luma of current encoding CU */
    u8                 qp_y;
    /* QP for chroma of current encoding CU */
    u8                 qp_u;
    u8                 qp_v;
    /* X address of current LCU */
    u16                x_lcu;
    /* Y address of current LCU */
    u16                y_lcu;
    /* X address of current CU in SCU unit */
    u16                x_scu;
    /* Y address of current CU in SCU unit */
    u16                y_scu;
    /* left pel position of current LCU */
    u16                x_pel;
    /* top pel position of current LCU */
    u16                y_pel;
    /* CU position in current frame in SCU unit */
    u32                scup;
    /* CU position in current LCU in SCU unit */
    u32                cup;
    /* CU depth */
    int                cud;
    /* neighbor CUs availability of current CU */
    u16                avail_cu;
    /* Left, right availability of current CU */
    u16                avail_lr;
    u16                bef_data_idx;
    /* CU mode */
    int                cu_mode;
    /* intra prediction mode */
    u8                 mpm[2]; /* mpm table pointer*/
    u8               * mpm_b_list;
    s8                 ipm[2];
    /* skip flag for MODE_INTER */
    u8                 skip_flag;
    /* width of current CU */
    u16                cuw;
    /* height of current CU */
    u16                cuh;
    /* log2 of cuw */
    u8                 log2_cuw;
    /* log2 of cuh */
    u8                 log2_cuh;
    /* number of non-zero coefficient */
    int                nnz[N_C];
    int                nnz_sub[N_C][MAX_SUB_TB_NUM];
    /* platform specific data, if needed */
    void             * pf;
    /* bitstream structure for RDO */
    XEVE_BSW           bs_temp;
    /* SBAC structure for full RDO */
    XEVE_SBAC          s_curr_best[NUM_CU_LOG2][NUM_CU_LOG2];
    XEVE_SBAC          s_next_best[NUM_CU_LOG2][NUM_CU_LOG2];
    XEVE_SBAC          s_temp_best;
    XEVE_SBAC          s_temp_best_merge;
    XEVE_SBAC          s_temp_run;
    XEVE_SBAC          s_temp_prev_comp_best;
    XEVE_SBAC          s_temp_prev_comp_run;
    XEVE_SBAC          s_curr_before_split[NUM_CU_LOG2][NUM_CU_LOG2];
    double             cost_best;
    u32                inter_satd;
    s32                dist_cu;
    s32                dist_cu_best; //dist of the best intra mode (note: only updated in intra coding now)

#if TRACE_ENC_CU_DATA
    u64  trace_idx;
#endif
    int                tile_num;
    /* current tile index */
    int                tile_idx;
    XEVE_CTX         * ctx;
    int                thread_cnt;
    TREE_CONS          tree_cons; //!< Tree status    
    u8                 ctx_flags[NUM_CNID]; 
    int                split_mode_child[4];
    int                parent_split_allow[6];    
    //one picture that arranges cu pixels and neighboring pixels for deblocking (just to match the interface of deblocking functions)
    s64                delta_dist[N_C];  //delta distortion from filtering (negative values mean distortion reduced)
    s64                dist_nofilt[N_C]; //distortion of not filtered samples
    s64                dist_filter[N_C]; //distortion of filtered samples
    /* RDOQ related variables*/
    int                rdoq_est_cbf_all[2];
    int                rdoq_est_cbf_luma[2];
    int                rdoq_est_cbf_cb[2];
    int                rdoq_est_cbf_cr[2];
    int                rdoq_est_sig_coeff[NUM_CTX_SIG_COEFF_FLAG][2];
    int                rdoq_est_gtx[NUM_CTX_GTX][2];
    int                rdoq_est_last_sig_coeff_x[NUM_CTX_LAST_SIG_COEFF][2];
    int                rdoq_est_last_sig_coeff_y[NUM_CTX_LAST_SIG_COEFF][2];
    s32                rdoq_est_run[NUM_CTX_CC_RUN][2];
    s32                rdoq_est_level[NUM_CTX_CC_LEVEL][2];
    s32                rdoq_est_last[NUM_CTX_CC_LAST][2];
};

/******************************************************************************
 * CONTEXT used for encoding process.
 *
 * All have to be stored are in this structure.
 *****************************************************************************/
struct _XEVE_CTX
{
    /* address of current input picture, ref_picture  buffer structure */
    XEVE_PICO        * pico_buf[XEVE_MAX_INBUF_CNT];
    /* address of current input picture buffer structure */
    XEVE_PICO        * pico;
    /* index of current input picture buffer in pico_buf[] */
    u8                 pico_idx;
    int                pico_max_cnt;
    /* magic code */
    u32                magic;
    /* XEVE identifier */
    XEVE               id;
    /* address of core structure */
    /* current input (original) image */
    XEVE_PIC           pic_o;
    /* address indicating current encoding, list0, list1 and original pictures */
    XEVE_PIC         * pic[PIC_D + 1]; /* the last one is for original */
    /* picture address for mode decision */
    XEVE_PIC         * pic_m;
    /* reference picture (0: foward, 1: backward) */
    XEVE_REFP          refp[MAX_NUM_REF_PICS][REFP_NUM];
    /* encoding parameter */
    XEVE_PARAM         param;
    /* bitstream structure */
    /* bitstream structure for RDO */
    /* sequnce parameter set */
    XEVE_SPS           sps;
    /* picture parameter set */
    XEVE_PPS           pps;
    XEVE_PPS           pps_array[64];
    /* adaptation parameter set */
    XEVE_APS_GEN     * aps_gen_array;
    XEVE_APS           aps;
    u8                 aps_counter;
    u8                 aps_temp;
    /* picture order count */
    XEVE_POC           poc;
    /* nal unit header */
    XEVE_NALU          nalu;
    /* slice header */
    XEVE_SH            sh;
    /* reference picture manager */
    XEVE_PM            rpm;
    /* create descriptor */
    XEVE_CDSC          cdsc;
    /* quantization value of current encoding slice */
    u8                 qp;
    /* offset value of alpha and beta for deblocking filter */
    u8                 deblock_alpha_offset;
    u8                 deblock_beta_offset;
    /* encoding picture width */
    u16                w;
    /* encoding picture height */
    u16                h;
    /* encoding picture width * height */
    u16                f;
    /* the picture order count of the previous Tid0 picture */
    u32                prev_pic_order_cnt_val;
    /* the picture order count msb of the previous Tid0 picture */
    u32                prev_pic_order_cnt_msb;
    /* the picture order count lsb of the previous Tid0 picture */
    u32                prev_pic_order_cnt_lsb;
    /* the decoding order count of the previous picture */
    u32                prev_doc_offset;
    /* current encoding picture count(This is not PicNum or FrameNum.
    Just count of encoded picture correctly) */
    u32                pic_cnt;
    /* current picture input count (only update when CTX0) */
    u32                pic_icnt;
    /* total input picture count (only used for bumping process) */
    u32                pic_ticnt;
    /* remaining pictures is encoded to p or b slice (only used for bumping process) */
    u8                 force_slice;
    /* ignored pictures for force slice count (unavailable pictures cnt in gop,\
    only used for bumping process) */
    u8                 force_ignored_cnt;
    /* initial frame return number(delayed input count) due to B picture or Forecast */
    u32                frm_rnum;
    /* current encoding slice number in one picture */
    int                slice_num;
    /* first mb number of current encoding slice in one picture */
    int                sl_first_mb;
    /* current slice type */
    u8                 slice_type;
    /* slice depth for current picture */
    u8                 slice_depth;
    /* flag whether current picture is refecened picture or not */
    u8                 slice_ref_flag;
    /* distance between ref pics in addition to closest ref ref pic in LD*/
    int                ref_pic_gap_length;
    /* maximum CU depth */
    u8                 max_cud;
    /* address of inbufs */
    XEVE_IMGB        * inbuf[XEVE_MAX_INBUF_CNT];
    /* last coded intra picture's picture order count */
    int                last_intra_poc;
    /* maximum CU width and height */
    u16                max_cuwh;
    /* log2 of maximum CU width and height */
    u8                 log2_max_cuwh;
    /* minimum CU width and height */
    u16                min_cuwh;
    /* log2 of minimum CU width and height */
    u8                 log2_min_cuwh;
    /* total count of remained LCU for encoding one picture. if a picture is
    encoded properly, this value should reach to zero */
    int                lcu_cnt;
    /* picture width in LCU unit */
    u16                w_lcu;
    /* picture height in LCU unit */
    u16                h_lcu;
    /* picture size in LCU unit (= w_lcu * h_lcu) */
    u32                f_lcu;
    /* picture width in SCU unit */
    u16                w_scu;
    /* picture height in SCU unit */
    u16                h_scu;
    /* picture size in SCU unit (= w_scu * h_scu) */
    u32                f_scu;
    /* log2 of SCU count in a LCU row */
    u8                 log2_culine;
    /* log2 of SCU count in a LCU (== log2_culine * 2) */
    u8                 log2_cudim;
    /* picture buffer allocator */
    PICBUF_ALLOCATOR   pa;
    /* MAPS *******************************************************************/
    /* CU map (width in SCU x height in SCU) of raster scan order in a frame */
    u32              * map_scu;
    /* cu data for current LCU */
    XEVE_CU_DATA     * map_cu_data;
    /* map for encoded motion vectors in SCU */
    s16             (* map_mv)[REFP_NUM][MV_D];
    /* map for encoded motion vectors in SCU */
    s16             (* map_unrefined_mv)[REFP_NUM][MV_D];
    /* map for reference indices */
    s8              (* map_refi)[REFP_NUM];
    /* map for intra pred mode */
    s8               * map_ipm;
    s8               * map_depth;
    XEVE_PIC         * pic_dbk;
    u32              * map_cu_mode;
    double             lambda[3];
    double             sqrt_lambda[3];
    double             dist_chroma_weight[2];
    /* rate control structure for one frame */
    XEVE_RCORE         * rcore;
    /* rate control for sequence */
    XEVE_RC          * rc;
    /* temporary tile bitstream store buffer if needed */
    u8               * bs_tbuf[MAX_NUM_TILES_ROW * MAX_NUM_TILES_COL];
    /* bs_tbuf byte size for one tile */
    int                bs_tbuf_size;
    THREAD_CONTROLLER * tc;
    POOL_THREAD        thread_pool[XEVE_MAX_TASK_CNT];
    int                parallel_rows;
    volatile s32     * sync_flag;
    SYNC_OBJ           sync_block;
    /* address of core structure */
    XEVE_CORE        * core[XEVE_MAX_TASK_CNT];
    XEVE_BSW           bs[XEVE_MAX_TASK_CNT];
    XEVE_SBAC          sbac_enc[XEVE_MAX_TASK_CNT];
    XEVE_MODE          mode[XEVE_MAX_TASK_CNT];
    XEVE_PINTRA        pintra[XEVE_MAX_TASK_CNT];
    XEVE_PINTER        pinter[XEVE_MAX_TASK_CNT];

    int   (*fn_ready)(XEVE_CTX * ctx);
    void  (*fn_flush)(XEVE_CTX * ctx);
    int   (*fn_enc)(XEVE_CTX * ctx, XEVE_BITB * bitb, XEVE_STAT * stat);
    int   (*fn_enc_header)(XEVE_CTX * ctx);
    int   (*fn_enc_pic_prepare)(XEVE_CTX * ctx, XEVE_BITB * bitb, XEVE_STAT * stat);
    int   (*fn_enc_pic)(XEVE_CTX * ctx, XEVE_BITB * bitb, XEVE_STAT * stat);
    int   (*fn_enc_pic_finish)(XEVE_CTX * ctx, XEVE_BITB * bitb, XEVE_STAT * stat);
    int   (*fn_push)(XEVE_CTX * ctx, XEVE_IMGB * img);
    int   (*fn_deblock)(XEVE_CTX * ctx, XEVE_PIC * pic, int tile_idx, int filter_across_boundary, XEVE_CORE * core);
    void  (*fn_picbuf_expand)(XEVE_CTX * ctx, XEVE_PIC * pic);
    int   (*fn_get_inbuf)(XEVE_CTX * ctx, XEVE_IMGB ** img);
    /* mode decision functions */
    int   (*fn_mode_init_tile)(XEVE_CTX * ctx, int tile_idx);
    int   (*fn_mode_init_lcu)(XEVE_CTX * ctx, XEVE_CORE * core);
    int   (*fn_mode_analyze_frame)(XEVE_CTX * ctx);
    int   (*fn_mode_analyze_lcu)(XEVE_CTX * ctx, XEVE_CORE * core);
    int   (*fn_mode_set_complexity)(XEVE_CTX * ctx, int complexity);
    void  (*fn_mode_copy_to_cu_data)(XEVE_CTX *ctx, XEVE_CORE *core, XEVE_MODE *mi, s16 coef_src[N_C][MAX_CU_DIM]);
    void  (*fn_mode_reset_intra)(XEVE_CORE *core);
    int   (*fn_mode_post_lcu)(XEVE_CTX * ctx, XEVE_CORE * core);
    void  (*fn_mode_rdo_dbk_map_set)(XEVE_CTX * ctx, XEVE_CORE *core, int log2_cuw, int log2_cuh, int cbf_l, int scup);
    void  (*fn_mode_rdo_bit_cnt_intra_dir)(XEVE_CTX * ctx, XEVE_CORE * core, int ipm);
    /* intra prediction functions */
    int   (*fn_pintra_init_tile)(XEVE_CTX * ctx, int tile_idx);
    int   (*fn_pintra_init_lcu)(XEVE_CTX * ctx, XEVE_CORE * core);
    double(*fn_pintra_analyze_cu)(XEVE_CTX *ctx, XEVE_CORE *core, int x, int y, int log2_cuw, int log2_cuh, XEVE_MODE *mi, s16 coef[N_C][MAX_CU_DIM], pel *rec[N_C], int s_rec[N_C]);
    int   (*fn_pintra_set_complexity)(XEVE_CTX * ctx, int complexity);
    /* inter prediction functions */
    int   (*fn_pinter_init_tile)(XEVE_CTX * ctx, int tile_idx);
    int   (*fn_pinter_init_lcu)(XEVE_CTX * ctx, XEVE_CORE * core);
    double(*fn_pinter_analyze_cu)(XEVE_CTX *ctx, XEVE_CORE *core, int x, int y, int log2_cuw, int log2_cuh, XEVE_MODE *mi, s16 coef[N_C][MAX_CU_DIM], pel *rec[N_C], int s_rec[N_C]);
    int   (*fn_pinter_set_complexity)(XEVE_CTX * ctx, int complexity);
    int   (*fn_loop_filter)(XEVE_CTX * ctx, XEVE_CORE * core);
    /* entropy coding functions */
    int   (*fn_eco_coef)(XEVE_CTX * ctx, XEVE_CORE * core, XEVE_BSW * bs, s16 coef[N_C][MAX_CU_DIM], u8 pred_mode, int enc_dqp, int b_no_cbf, int run_stats);
    void  (*fn_rdo_intra_ext)(XEVE_CTX * ctx, XEVE_CORE * core);
    void  (*fn_rdo_intra_ext_c)(XEVE_CTX * ctx, XEVE_CORE * core);
    int   (*fn_eco_pic_signature)(XEVE_CTX * ctx, XEVE_BSW * bs, u8 pic_sign[N_C][16]);
    int   (*fn_eco_sps)(XEVE_BSW * bs, XEVE_SPS * sps);
    int   (*fn_eco_pps)(XEVE_BSW * bs, XEVE_SPS * sps, XEVE_PPS * pps);
    int   (*fn_eco_sh)(XEVE_BSW * bs, XEVE_SPS * sps, XEVE_PPS * pps, XEVE_SH * sh, int nut);
    int   (*fn_eco_split_mode)(XEVE_BSW *bs, XEVE_CTX *c, XEVE_CORE *core, int cud, int cup, int cuw, int cuh, int lcu_s, int x, int y);
    void  (*fn_eco_sbac_reset)(XEVE_SBAC *sbac, u8 slice_type, u8 slice_qp, int sps_cm_init_flag);
    void  (*fn_itdp)(XEVE_CTX * ctx, XEVE_CORE * core, s16 coef[N_C][MAX_CU_DIM], int nnz_sub[N_C][MAX_SUB_TB_NUM]);
    int   (*fn_tq)(XEVE_CTX * ctx, XEVE_CORE * core, s16 coef[N_C][MAX_CU_DIM], int log2_cuw, int log2_cuh, int slice_type, int nnz[N_C], int is_intra, int run_stats);
    int   (*fn_rdoq_set_ctx_cc)(XEVE_CORE * core, int ch_type, int prev_level);
    void  (*fn_recon)(XEVE_CTX * ctx, XEVE_CORE * core, s16 *coef, pel *pred, int is_coef, int cuw, int cuh, int s_rec, pel *rec, int bit_depth);
    void  (*fn_deblock_unit)(XEVE_CTX * ctx, XEVE_PIC * pic, int x, int y, int cuw, int cuh, int is_hor_edge, XEVE_CORE * core, int boundary_filtering);
    /* platform specific data, if needed */
    void             * pf;

    /* Tile information for each index */
    XEVE_TILE        * tile;
    /* Total number of tiles in the picture*/
    u32                tile_cnt;

    /* tile index map (width in SCU x height in SCU) of
       raster scan order in a frame */
    u8               * map_tidx;
    u8                 tile_to_slice_map[MAX_NUM_TILES_COL * MAX_NUM_TILES_ROW];
    u8                 tiles_in_slice[MAX_NUM_TILES_COL * MAX_NUM_TILES_ROW];
    u8                 tile_order[MAX_NUM_TILES_COL * MAX_NUM_TILES_ROW];

};

#define PIC_CURR(ctx)             ((ctx)->pic[PIC_IDX_CURR])
#define PIC_ORIG(ctx)             ((ctx)->pic[PIC_IDX_ORIG])
#define PIC_MODE(ctx)             ((ctx)->pic[PIC_IDX_MODE])

int  xeve_platform_init(XEVE_CTX * ctx);
void xeve_platform_deinit(XEVE_CTX * ctx);
int  xeve_pic_prepare(XEVE_CTX * ctx, XEVE_BITB * bitb, XEVE_STAT * stat);
int  xeve_pic_finish(XEVE_CTX * ctx, XEVE_BITB * bitb, XEVE_STAT * stat);
int  xeve_pic(XEVE_CTX * ctx, XEVE_BITB * bitb, XEVE_STAT * stat);
int  xeve_deblock(XEVE_CTX * ctx, XEVE_PIC * pic, int tile_idx, int filter_across_boundary, XEVE_CORE * core);
int  xeve_enc(XEVE_CTX * ctx, XEVE_BITB * bitb, XEVE_STAT * stat);
int  xeve_push_frm(XEVE_CTX * ctx, XEVE_IMGB * img);
int  xeve_ready(XEVE_CTX * ctx);
void xeve_flush(XEVE_CTX * ctx);
int  xeve_picbuf_get_inbuf(XEVE_CTX * ctx, XEVE_IMGB ** img);

typedef struct _ADAPTIVE_LOOP_FILTER ADAPTIVE_LOOP_FILTER;
typedef struct _ALF_FILTER_SHAPE ALF_FILTER_SHAPE;
typedef struct _ALF_SLICE_PARAM ALF_SLICE_PARAM;

#include "xeve_eco.h"
#include "xeve_mode.h"
#include "xeve_tq.h"
#include "xeve_pred.h"
#include "xeve_df.h"

#endif /* _XEVE_TYPE_H_ */
