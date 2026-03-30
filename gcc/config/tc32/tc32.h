/* TC32 target definitions for GCC 4.5.1.  */

#ifndef GCC_TC32_H
#define GCC_TC32_H

#define TARGET_VERSION fprintf (stderr, " (Telink TC32)")

extern void tc32_override_options (void);

#define OVERRIDE_OPTIONS tc32_override_options ()

#define TARGET_CPU_CPP_BUILTINS()         \
  do                                      \
    {                                     \
      builtin_define ("__tc32__");        \
      builtin_assert ("cpu=tc32");        \
      builtin_assert ("machine=tc32");    \
    }                                     \
  while (0)

#define BITS_BIG_ENDIAN 0
#define BYTES_BIG_ENDIAN 0
#define WORDS_BIG_ENDIAN 0

#define BITS_PER_UNIT 8
#define BITS_PER_WORD 32
#define UNITS_PER_WORD 4
#define POINTER_SIZE 32
#define PARM_BOUNDARY 32
#define STACK_BOUNDARY 32
#define FUNCTION_BOUNDARY 32
#define BIGGEST_ALIGNMENT 32
#define EMPTY_FIELD_BOUNDARY 32
#define BIGGEST_FIELD_ALIGNMENT 64
#define STRUCTURE_SIZE_BOUNDARY 32
#define PCC_BITFIELD_TYPE_MATTERS 1
#define STRICT_ALIGNMENT 1
#define DEFAULT_SIGNED_CHAR 0
#define NO_FUNCTION_CSE 1

/* Match ARM/Thumb aggregate object alignment: keep arrays, unions and
   records word-aligned for global emission even under -fpack-struct.
   This affects COMMON/BSS alignment, not field layout.  */
#define DATA_ALIGNMENT(EXP, ALIGN)                                      \
  ((((ALIGN) < BITS_PER_WORD)                                           \
    && (TREE_CODE (EXP) == ARRAY_TYPE                                   \
        || TREE_CODE (EXP) == UNION_TYPE                                \
        || TREE_CODE (EXP) == RECORD_TYPE)) ? BITS_PER_WORD : (ALIGN))

#define LOCAL_ALIGNMENT(EXP, ALIGN) DATA_ALIGNMENT ((EXP), (ALIGN))

#define INT_TYPE_SIZE 32
#define SHORT_TYPE_SIZE 16
#define LONG_TYPE_SIZE 32
#define LONG_LONG_TYPE_SIZE 64

#define SIZE_TYPE "unsigned int"
#define PTRDIFF_TYPE "int"

#define Pmode SImode
#define FUNCTION_MODE QImode

#define PROMOTE_MODE(MODE, UNSIGNEDP, TYPE)           \
  do                                                  \
    {                                                 \
      if (GET_MODE_CLASS (MODE) == MODE_INT           \
          && GET_MODE_SIZE (MODE) < UNITS_PER_WORD)   \
        (MODE) = SImode;                              \
    }                                                 \
  while (0)

#define LOAD_EXTEND_OP(MODE) ZERO_EXTEND

#define OPTIMIZATION_OPTIONS(LEVEL, SIZE) \
  tc32_optimization_options ((LEVEL), (SIZE))

#define ORDER_REGS_FOR_LOCAL_ALLOC \
  tc32_order_regs_for_local_alloc ()

#define FIRST_PSEUDO_REGISTER 18

#define TC32_R0_REGNUM 0
#define TC32_R1_REGNUM 1
#define TC32_R2_REGNUM 2
#define TC32_R3_REGNUM 3
#define TC32_R4_REGNUM 4
#define TC32_R5_REGNUM 5
#define TC32_R6_REGNUM 6
#define TC32_R7_REGNUM 7
#define TC32_IP_REGNUM 12
#define TC32_SP_REGNUM 13
#define TC32_LR_REGNUM 14
#define TC32_PC_REGNUM 15
#define TC32_FP_REGNUM 16
#define TC32_AP_REGNUM 17

#define STACK_POINTER_REGNUM TC32_SP_REGNUM
#define FRAME_POINTER_REGNUM TC32_FP_REGNUM
#define ARG_POINTER_REGNUM TC32_AP_REGNUM
#define HARD_FRAME_POINTER_REGNUM TC32_R7_REGNUM
#define RETURN_ADDRESS_POINTER_REGNUM TC32_LR_REGNUM

#define LAST_ARG_REGNUM TC32_R3_REGNUM
#define LAST_LO_REGNUM TC32_R7_REGNUM

#define FIXED_REGISTERS                           \
{                                                 \
  0, 0, 0, 0, 0, 0, 0, 0,                        \
  0, 0, 0, 0, 1, 1, 1, 1,                        \
  1, 1                                           \
}

#define CALL_USED_REGISTERS                       \
{                                                 \
  1, 1, 1, 1, 0, 0, 0, 0,                        \
  0, 0, 0, 0, 1, 1, 1, 1,                        \
  1, 1                                           \
}

#define REGISTER_NAMES                            \
{                                                 \
  "r0", "r1", "r2", "r3",                        \
  "r4", "r5", "r6", "r7",                        \
  "r8", "r9", "r10", "r11",                      \
  "r12", "sp", "lr", "pc",                       \
  "frame",                                       \
  "arg"                                          \
}

#define REG_ALLOC_ORDER                           \
{                                                 \
  4, 5, 6, 7, 0, 1, 2, 3,                        \
  14, 13, 12, 11, 10, 9, 8, 15,                  \
  16, 17                                         \
}

#define HARD_REGNO_NREGS(REGNO, MODE) \
  ((GET_MODE_SIZE (MODE) + UNITS_PER_WORD - 1) / UNITS_PER_WORD)

#define HARD_REGNO_MODE_OK(REGNO, MODE) ((REGNO) < FIRST_PSEUDO_REGISTER)

#define MODES_TIEABLE_P(MODE1, MODE2) \
  (GET_MODE_CLASS (MODE1) == MODE_INT && GET_MODE_CLASS (MODE2) == MODE_INT)

enum reg_class
{
  NO_REGS,
  LO_REGS,
  HI_REGS,
  STACK_REG,
  BASE_REGS,
  CORE_REGS,
  GENERAL_REGS,
  SPECIAL_REGS,
  ALL_REGS,
  LIM_REG_CLASSES
};

#define N_REG_CLASSES ((int) LIM_REG_CLASSES)
#define REG_CLASS_NAMES \
{ \
  "NO_REGS", "LO_REGS", "HI_REGS", "STACK_REG", "BASE_REGS", "CORE_REGS", \
  "GENERAL_REGS", "SPECIAL_REGS", "ALL_REGS" \
}
#define REG_CLASS_CONTENTS \
{ \
  { 0x00000 }, \
  { 0x000ff }, \
  { 0x0ff00 }, \
  { 0x02000 }, \
  { 0x020ff }, \
  { 0x00fff }, \
  { 0x170ff }, \
  { 0x37000 }, \
  { 0x3ffff } \
}

#define REGNO_REG_CLASS(REGNO) \
  ((REGNO) <= LAST_LO_REGNUM ? LO_REGS \
      : ((REGNO) == STACK_POINTER_REGNUM ? STACK_REG \
      : ((REGNO) == TC32_IP_REGNUM || (REGNO) == TC32_FP_REGNUM \
         || (REGNO) == TC32_AP_REGNUM \
         || (REGNO) == TC32_LR_REGNUM) \
      ? SPECIAL_REGS : ((REGNO) < FIRST_PSEUDO_REGISTER ? HI_REGS : GENERAL_REGS)))

#define BASE_REG_CLASS BASE_REGS
#define INDEX_REG_CLASS LO_REGS

/* Like Thumb-1, narrow accesses cannot freely use SP/high-reg bases.
   Be conservative unless the mode is full-word.  */
#define MODE_BASE_REG_CLASS(MODE) \
  (((MODE) == SImode) ? BASE_REGS : LO_REGS)

/* TC32 does not support SP+reg addressing either, so keep reg+reg bases
   in the low-register class.  */
#define MODE_BASE_REG_REG_CLASS(MODE) BASE_REG_CLASS

#define REGNO_OK_FOR_BASE_P(REGNO) \
  ((REGNO) < 8 || ((unsigned) reg_renumber[REGNO]) < 8)

#define REGNO_OK_FOR_INDEX_P(REGNO) REGNO_OK_FOR_BASE_P (REGNO)

#define THUMB1_REGNO_MODE_OK_FOR_BASE_P(REGNO, MODE)          \
  (((REGNO) <= LAST_LO_REGNUM)                                \
   || (REGNO) == HARD_FRAME_POINTER_REGNUM                    \
   || (GET_MODE_SIZE (MODE) >= 4                              \
       && (REGNO) == STACK_POINTER_REGNUM))

#define CLASS_MAX_NREGS(CLASS, MODE) \
  ((GET_MODE_SIZE (MODE) + UNITS_PER_WORD - 1) / UNITS_PER_WORD)

#define STACK_GROWS_DOWNWARD 1
#define FRAME_GROWS_DOWNWARD 1
#define STARTING_FRAME_OFFSET 0
#define FIRST_PARM_OFFSET(FNDECL) 0
#define STACK_POINTER_OFFSET 0
#define ACCUMULATE_OUTGOING_ARGS 1
/* Make DWARF use the frame-pointer path instead of the raw arg pointer.
   The latter is not guaranteed to be eliminated when dwarf2out computes
   frame-base displacements.  */
#define FRAME_POINTER_CFA_OFFSET(FNDECL) 0

#define ELIMINABLE_REGS \
{ \
  { ARG_POINTER_REGNUM, STACK_POINTER_REGNUM }, \
  { ARG_POINTER_REGNUM, FRAME_POINTER_REGNUM }, \
  { ARG_POINTER_REGNUM, HARD_FRAME_POINTER_REGNUM }, \
  { FRAME_POINTER_REGNUM, HARD_FRAME_POINTER_REGNUM }, \
  { FRAME_POINTER_REGNUM, STACK_POINTER_REGNUM } \
}

#define INITIAL_ELIMINATION_OFFSET(FROM, TO, OFFSET) \
  ((OFFSET) = tc32_initial_elimination_offset ((FROM), (TO)))

#define INIT_EXPANDERS \
  tc32_init_expanders ()

typedef struct cumulative_args
{
  int nregs;
  int can_split;
} CUMULATIVE_ARGS;

#ifndef IN_LIBGCC2
extern void tc32_optimization_options (int, int);
extern void tc32_order_regs_for_local_alloc (void);
extern rtx tc32_function_arg (CUMULATIVE_ARGS, int, tree, int);
extern void tc32_function_arg_advance (CUMULATIVE_ARGS *, int, tree, int);
extern rtx tc32_function_value (const_tree, const_tree);
extern int tc32_return_in_memory (tree);
extern HOST_WIDE_INT tc32_initial_elimination_offset (int, int);
extern void tc32_init_expanders (void);
extern int tc32_reg_ok_for_base_p (rtx, int);
extern bool tc32_legitimate_address_p (int, rtx, int);
extern void tc32_print_operand (FILE *, rtx, int);
extern void tc32_print_operand_address (FILE *, rtx);
extern const char *tc32_output_movsi_core_from_special (rtx *);
extern const char *tc32_output_movsi_low_from_special (rtx *);
extern const char *thumb1_output_casesi (rtx *);
extern void thumb1_expand_prologue (void);
extern void thumb1_expand_epilogue (void);
extern void thumb1_final_prescan_insn (rtx);
extern int thumb_shiftable_const (unsigned HOST_WIDE_INT);
extern rtx thumb_legitimize_reload_address (rtx *, int, int, int, int);
extern void thumb_expand_movmemqi (rtx *);
extern void thumb_reload_out_hi (rtx *);
extern void thumb_reload_in_hi (rtx *);
extern const char *thumb_output_move_mem_multiple (int, rtx *);
extern const char *thumb_call_via_reg (rtx);
extern const char *thumb_load_double_from_address (rtx *);
extern const char *tc32_output_loadsi_stack (rtx *);
extern const char *tc32_output_loadsi_special_neg_offset (rtx *);
extern const char *tc32_output_storesi_stack (rtx *);
extern const char *tc32_output_movsi_memmem (rtx *);
extern const char *tc32_output_movsi (rtx *);
extern const char *tc32_output_movhi (rtx *);
extern const char *tc32_output_movqi (rtx *);
extern const char *tc32_output_addsi3 (rtx *);
extern const char *tc32_output_addsi3_sp_reg (rtx *);
extern const char *tc32_output_addsi3_special_neg_mem (rtx *);
extern const char *tc32_output_subsi3 (rtx *);
extern const char *tc32_output_subsi3_sp_reg (rtx *);
extern const char *tc32_output_addsi3_symbol (rtx *);
extern const char *tc32_output_subsi3_symbol (rtx *);
extern const char *tc32_output_cbranch (rtx, int, int);
extern const char *tc32_output_tbit_cbranch (rtx, int, int);
extern const char *tc32_output_jump (rtx, int);
extern HOST_WIDE_INT tc32_simm32_intval (rtx);
extern HOST_WIDE_INT thumb_compute_initial_elimination_offset (unsigned int,
                                                               unsigned int);
extern const char *thumb_unexpanded_epilogue (void);
extern void thumb_set_return_address (rtx, rtx);
extern void thumb_set_frame_pointer (void);
extern void thumb1_output_function_prologue (FILE *, HOST_WIDE_INT);
#endif

#define INIT_CUMULATIVE_ARGS(CUM, FNTYPE, LIBNAME, INDIRECT, N_NAMED_ARGS) \
  do                                                                       \
    {                                                                      \
      (CUM).nregs = 0;                                                     \
      (CUM).can_split = 1;                                                 \
    }                                                                      \
  while (0)

#define FUNCTION_ARG_REGNO_P(N) ((N) >= TC32_R0_REGNUM && (N) <= TC32_R3_REGNUM)
#define FUNCTION_VALUE_REGNO_P(N) ((N) == TC32_R0_REGNUM)

#define EH_RETURN_DATA_REGNO(N) (((N) < 2) ? (N) : INVALID_REGNUM)
#define TC32_EH_STACKADJ_REGNUM TC32_R2_REGNUM
#define EH_RETURN_STACKADJ_RTX \
  gen_rtx_REG (SImode, TC32_EH_STACKADJ_REGNUM)

#define FUNCTION_ARG(CUM, MODE, TYPE, NAMED) \
  tc32_function_arg ((CUM), (MODE), (TYPE), (NAMED))

#define FUNCTION_ARG_ADVANCE(CUM, MODE, TYPE, NAMED) \
  tc32_function_arg_advance (&(CUM), (MODE), (TYPE), (NAMED))

#define FUNCTION_VALUE(VALTYPE, FUNC) tc32_function_value ((VALTYPE), (FUNC))
#define LIBCALL_VALUE(MODE) gen_rtx_REG ((MODE), TC32_R0_REGNUM)
#define RETURN_IN_MEMORY(TYPE) tc32_return_in_memory (TYPE)
#define RETURN_POPS_ARGS(FUNDECL, FUNTYPE, SIZE) 0
#define SMALL_REGISTER_CLASSES 1
#define PREFERRED_RELOAD_CLASS(X, CLASS)                         \
  (((CLASS) == GENERAL_REGS                                      \
    || (CLASS) == HI_REGS                                        \
    || (CLASS) == NO_REGS                                        \
    || (CLASS) == STACK_REG)                                     \
   ? LO_REGS : (CLASS))

#define SECONDARY_INPUT_RELOAD_CLASS(CLASS, MODE, X)                     \
  ((CLASS) == NO_REGS                                                    \
   ? NO_REGS                                                             \
   : (CLASS) != LO_REGS && (CLASS) != BASE_REGS                          \
   ? ((true_regnum (X) == -1                                             \
       || (true_regnum (X) + HARD_REGNO_NREGS (0, MODE) > 8))           \
      ? LO_REGS : NO_REGS)                                              \
   : NO_REGS)

#define SECONDARY_OUTPUT_RELOAD_CLASS(CLASS, MODE, X)                    \
  ((CLASS) == NO_REGS                                                    \
   ? NO_REGS                                                             \
   : (CLASS) != LO_REGS && (CLASS) != BASE_REGS                          \
   ? ((true_regnum (X) == -1                                             \
       || (true_regnum (X) + HARD_REGNO_NREGS (0, MODE) > 8))           \
      ? LO_REGS : NO_REGS)                                              \
   : NO_REGS)

#define TC32_CONST_SI(VALUE) trunc_int_for_mode ((VALUE), SImode)
#define CONST_OK_FOR_I(VALUE) \
  ((unsigned HOST_WIDE_INT) TC32_CONST_SI (VALUE) <= 255)
#define CONST_OK_FOR_J(VALUE) \
  (TC32_CONST_SI (VALUE) >= -255 && TC32_CONST_SI (VALUE) <= -1)
#define CONST_OK_FOR_L(VALUE) \
  (TC32_CONST_SI (VALUE) >= -7 && TC32_CONST_SI (VALUE) <= 7)
#define CONST_OK_FOR_M(VALUE) \
  (TC32_CONST_SI (VALUE) >= 0 && TC32_CONST_SI (VALUE) <= 1020 \
   && (TC32_CONST_SI (VALUE) & 3) == 0)
#define CONST_OK_FOR_O(VALUE) \
  (TC32_CONST_SI (VALUE) >= -508 && TC32_CONST_SI (VALUE) <= 508 \
   && (TC32_CONST_SI (VALUE) & 3) == 0)
#define CONST_OK_FOR_LETTER_P(VALUE, C) \
  ((C) == 'I' ? CONST_OK_FOR_I (VALUE) \
   : ((C) == 'J' ? CONST_OK_FOR_J (VALUE) \
      : ((C) == 'L' ? CONST_OK_FOR_L (VALUE) \
         : ((C) == 'M' ? CONST_OK_FOR_M (VALUE) \
            : ((C) == 'O' ? CONST_OK_FOR_O (VALUE) : 0)))))
#define CONST_OK_FOR_CONSTRAINT_P(VALUE, C, STR) \
  ((C) == 'P' \
   ? ((STR)[1] == 'a' \
      ? (TC32_CONST_SI (VALUE) >= -510 && TC32_CONST_SI (VALUE) <= 510 \
         && (TC32_CONST_SI (VALUE) > 255 || TC32_CONST_SI (VALUE) < -255)) \
      : ((STR)[1] == 'b' \
         ? (TC32_CONST_SI (VALUE) >= -262 && TC32_CONST_SI (VALUE) <= 262 \
            && (TC32_CONST_SI (VALUE) > 255 || TC32_CONST_SI (VALUE) < -255)) \
         : 0)) \
   : CONST_OK_FOR_LETTER_P ((VALUE), (C)))
#define CONST_DOUBLE_OK_FOR_CONSTRAINT_P(VALUE, C, STR) 0
#define REG_CLASS_FROM_LETTER(C) \
  ((C) == 'r' ? GENERAL_REGS \
   : ((C) == 'l' ? LO_REGS \
      : ((C) == 'h' ? HI_REGS \
         : ((C) == 'c' ? ALL_REGS \
            : ((C) == 'x' ? SPECIAL_REGS \
               : ((C) == 'k' ? STACK_REG \
                  : ((C) == 'b' ? BASE_REGS : NO_REGS)))))))
#define REG_CLASS_FROM_CONSTRAINT(C, STR) REG_CLASS_FROM_LETTER (C)
#define CONSTRAINT_LEN(C, STR) DEFAULT_CONSTRAINT_LEN ((C), (STR))
#define EXTRA_ADDRESS_CONSTRAINT(C, STR) 0

#define MOVE_MAX 4
#define FUNCTION_PROFILER(FILE, LABELNO)
#define TRULY_NOOP_TRUNCATION(OUTPREC, INPREC) 1
#define LEGITIMATE_CONSTANT_P(X) 1
#define CONSTANT_POOL_BEFORE_FUNCTION 0
#define CASE_VECTOR_MODE SImode
#define MAX_REGS_PER_ADDRESS 2
#define SLOW_BYTE_ACCESS 0

/* Match Thumb-1 heuristics: branches are not free, so generic optimizers
   should not over-prefer branch-heavy shapes in large control-flow regions. */
#define BRANCH_COST(speed_p, predictable_p) (optimize > 0 ? 2 : 0)
#define TRAMPOLINE_SIZE 0
#define TRAMPOLINE_ALIGNMENT 32
#define TARGET_DISABLE_POSTRELOAD_CSE 1

#ifndef REG_OK_STRICT
#define TC32_REG_STRICT_P 0
#else
#define TC32_REG_STRICT_P 1
#endif

#define GO_IF_LEGITIMATE_ADDRESS(MODE, X, LABEL)                      \
  do                                                                  \
    {                                                                 \
      if (tc32_legitimate_address_p ((MODE), (X), TC32_REG_STRICT_P)) \
        goto LABEL;                                                   \
    }                                                                 \
  while (0)

#define THUMB_LEGITIMIZE_RELOAD_ADDRESS(X, MODE, OPNUM, TYPE, IND_L, WIN) \
  do                                                                       \
    {                                                                      \
      rtx new_x = thumb_legitimize_reload_address (&(X), (MODE), (OPNUM),  \
                                                   (TYPE), (IND_L));       \
      if (new_x)                                                           \
        {                                                                  \
          (X) = new_x;                                                     \
          goto WIN;                                                        \
        }                                                                  \
    }                                                                      \
  while (0)

#define LEGITIMIZE_RELOAD_ADDRESS(X, MODE, OPNUM, TYPE, IND_LEVELS, WIN) \
  THUMB_LEGITIMIZE_RELOAD_ADDRESS (X, MODE, OPNUM, TYPE, IND_LEVELS, WIN)

#define REG_OK_FOR_BASE_P(X) tc32_reg_ok_for_base_p ((X), TC32_REG_STRICT_P)
#define REG_OK_FOR_INDEX_P(X) tc32_reg_ok_for_base_p ((X), TC32_REG_STRICT_P)

#define ASM_COMMENT_START "@"
#define ASM_APP_ON ""
#define ASM_APP_OFF ""
#define TEXT_SECTION_ASM_OP "\t.text"
#define DATA_SECTION_ASM_OP "\t.data"
#define BSS_SECTION_ASM_OP "\t.bss"
#define GLOBAL_ASM_OP "\t.global\t"
#undef TYPE_OPERAND_FMT
#define TYPE_OPERAND_FMT "%%%s"
#undef ASM_DECLARE_FUNCTION_NAME
#define ASM_DECLARE_FUNCTION_NAME(FILE, NAME, DECL)                     \
  do                                                                    \
    {                                                                   \
      fprintf ((FILE), "\t.code\t16\n");                                \
      fprintf ((FILE), "\t.tc32_func\n");                               \
      ASM_OUTPUT_TYPE_DIRECTIVE ((FILE), (NAME), "function");           \
      ASM_DECLARE_RESULT ((FILE), DECL_RESULT (DECL));                  \
      ASM_OUTPUT_LABEL ((FILE), (NAME));                                \
    }                                                                   \
  while (0)
#define ASM_OUTPUT_ALIGN(FILE, LOG) \
  if ((LOG) != 0) fprintf ((FILE), "\t.align\t%d\n", (LOG))

#define ASM_OUTPUT_ADDR_VEC_ELT(STREAM, VALUE) \
  fprintf ((STREAM), "\t.word\t.L%d\n", (VALUE))

#define ASM_OUTPUT_ADDR_DIFF_ELT(STREAM, BODY, VALUE, REL) \
  fprintf ((STREAM), "\t.word\t.L%d-.L%d\n", (VALUE), (REL))

#define PRINT_OPERAND(STREAM, X, CODE) \
  tc32_print_operand ((STREAM), (X), (CODE))
#define PRINT_OPERAND_ADDRESS(STREAM, X) \
  tc32_print_operand_address ((STREAM), (X))

#define FINAL_PRESCAN_INSN(INSN, OPVEC, NOPERANDS) \
  thumb1_final_prescan_insn (INSN)

#endif
