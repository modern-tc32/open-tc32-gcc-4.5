/* Minimal experimental tc32 backend.  */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "rtl.h"
#include "regs.h"
#include "hard-reg-set.h"
#include "basic-block.h"
#include "real.h"
#include "insn-config.h"
#include "conditions.h"
#include "insn-flags.h"
#include "insn-attr.h"
#include "tree.h"
#include "tm_p.h"
#include "flags.h"
#include "expr.h"
#include "output.h"
#include "function.h"
#include "df.h"
#include "recog.h"
#include "reload.h"
#include "toplev.h"
#include "target.h"
#include "target-def.h"
#include "langhooks.h"

struct tc32_stack_offsets
{
  int saved_args;
  int saved_regs;
  int soft_frame;
  int locals_base;
  int outgoing_args;
  unsigned int saved_regs_mask;
};

static bool tc32_return_in_memory_1 (const_tree, const_tree);
static rtx tc32_function_value_1 (const_tree, const_tree, bool);
static bool tc32_can_eliminate_1 (const int, const int);
static bool tc32_legitimate_address_p_1 (enum machine_mode, rtx, bool);
static rtx tc32_legitimize_address_1 (rtx, rtx, enum machine_mode);
void tc32_override_options (void);
static bool tc32_has_incoming_stack_args_p (void);
static bool tc32_frame_pointer_required (void);
static enum machine_mode tc32_promote_function_mode (const_tree,
                                                     enum machine_mode,
                                                     int *, const_tree, int);
static bool tc32_promote_prototypes (const_tree);
static bool tc32_pass_by_reference (CUMULATIVE_ARGS *, enum machine_mode,
                                    const_tree, bool);
static int tc32_arg_partial_bytes (CUMULATIVE_ARGS *, enum machine_mode,
                                   tree, bool);
void tc32_init_expanders (void);
static bool tc32_rtx_costs (rtx, int, int, int *, bool);
static int tc32_address_cost (rtx, bool);
static void tc32_file_start (void);
static void tc32_file_end (void);
static section *tc32_function_rodata_section (tree);
static int tc32_bit_count (unsigned long);
static bool tc32_needs_doubleword_align (enum machine_mode, tree);
static inline int thumb1_rtx_costs (rtx, enum rtx_code, enum rtx_code);
static int number_of_first_bit_set (unsigned);
static bool tc32_has_call_insn (void);
static bool tc32_simple_leaf_function_p (void);
static bool tc32_real_frame_pointer_needed (void);
static bool tc32_hard_frame_pointer_available_p (void);
static int tc32_frame_access_base_regno (void);
static int tc32_resolve_stack_base_offset (enum machine_mode, int,
                                           HOST_WIDE_INT, HOST_WIDE_INT *);
static int thumb_far_jump_used_p (void);
static bool thumb_force_lr_save (void);
static unsigned tc32_size_return_regs (void);
static void tc32_output_chunked_adjust (FILE *, const char *, int, int,
                                        HOST_WIDE_INT);
static int tc32_split_shiftable_const (HOST_WIDE_INT, unsigned HOST_WIDE_INT *,
                                       int *);
static void tc32_output_sp_adjust (FILE *, HOST_WIDE_INT, unsigned long);
static bool tc32_legitimate_stack_mem_offset_p (enum machine_mode, int,
                                                HOST_WIDE_INT);
static bool tc32_constant_pool_ref_p (rtx);
static rtx tc32_get_pool_ref_constant (rtx);
static rtx tc32_choose_low_scratch (int, int);
static bool tc32_hi_reg_p (rtx);
static rtx tc32_copy_to_low_reg (rtx, int, int);
static void tc32_emit_chunked_addsub (const char *, rtx *, HOST_WIDE_INT);
static void tc32_emit_address_to_reg (rtx, rtx, int);
static void tc32_emit_lowreg_load (const char *, rtx, rtx);
static void tc32_emit_lowreg_store (const char *, rtx, rtx);
static int tc32_resolve_arg_pointer_offset (enum machine_mode, HOST_WIDE_INT,
                                            HOST_WIDE_INT *);
static void thumb_pushpop (FILE *, unsigned long, int, int *, unsigned long);
static void thumb_exit (FILE *, int);
void thumb1_expand_prologue (void);
void thumb1_expand_epilogue (void);

static struct tc32_stack_offsets *thumb_get_frame_offsets (void);
static int thumb_find_work_register (unsigned long);
static unsigned long thumb1_compute_save_reg_mask (void);
static int thumb1_base_register_rtx_p (rtx, enum machine_mode, int);
inline static int thumb1_index_register_rtx_p (rtx, int);
static int thumb1_legitimate_address_p (enum machine_mode, rtx, int);
static int thumb_legitimate_offset_p (enum machine_mode, HOST_WIDE_INT);
static rtx thumb_legitimize_address (rtx, rtx, enum machine_mode);
void thumb1_final_prescan_insn (rtx);
int thumb_shiftable_const (unsigned HOST_WIDE_INT);
rtx thumb_legitimize_reload_address (rtx *, int, int, int, int);
void thumb_expand_movmemqi (rtx *);
void thumb_reload_out_hi (rtx *);
void thumb_reload_in_hi (rtx *);
const char *thumb_output_move_mem_multiple (int, rtx *);
const char *thumb_call_via_reg (rtx);
const char *thumb_load_double_from_address (rtx *);
const char *tc32_output_loadsi_special_neg_offset (rtx *);
const char *tc32_output_movsi (rtx *);
const char *tc32_output_movsi_core_from_special (rtx *);
const char *tc32_output_movsi_low_from_special (rtx *);
const char *tc32_output_movhi (rtx *);
const char *tc32_output_movqi (rtx *);
const char *tc32_output_addsi3 (rtx *);
const char *tc32_output_addsi3_special_neg_mem (rtx *);
const char *tc32_output_subsi3 (rtx *);
const char *tc32_output_addsi3_symbol (rtx *);
const char *tc32_output_subsi3_symbol (rtx *);
const char *tc32_output_cbranch (rtx, int, int);
const char *tc32_output_jump (rtx, int);
const char *tc32_output_cond_jump_tail (rtx, int);
HOST_WIDE_INT tc32_simm32_intval (rtx);
HOST_WIDE_INT thumb_compute_initial_elimination_offset (unsigned int,
                                                        unsigned int);
const char *thumb_unexpanded_epilogue (void);
void thumb_set_return_address (rtx, rtx);
void thumb_set_frame_pointer (void);
void thumb1_output_function_prologue (FILE *, HOST_WIDE_INT);
void tc32_order_regs_for_local_alloc (void);
static int tc32_arg_words (enum machine_mode, tree);

#undef TARGET_RETURN_IN_MEMORY
#define TARGET_RETURN_IN_MEMORY tc32_return_in_memory_1
#undef TARGET_FUNCTION_VALUE
#define TARGET_FUNCTION_VALUE tc32_function_value_1
#undef TARGET_CAN_ELIMINATE
#define TARGET_CAN_ELIMINATE tc32_can_eliminate_1
#undef TARGET_LEGITIMATE_ADDRESS_P
#define TARGET_LEGITIMATE_ADDRESS_P tc32_legitimate_address_p_1
#undef TARGET_LEGITIMIZE_ADDRESS
#define TARGET_LEGITIMIZE_ADDRESS tc32_legitimize_address_1
#undef TARGET_FRAME_POINTER_REQUIRED
#define TARGET_FRAME_POINTER_REQUIRED tc32_frame_pointer_required
#undef TARGET_PROMOTE_FUNCTION_MODE
#define TARGET_PROMOTE_FUNCTION_MODE tc32_promote_function_mode
#undef TARGET_PROMOTE_PROTOTYPES
#define TARGET_PROMOTE_PROTOTYPES tc32_promote_prototypes
#undef TARGET_PASS_BY_REFERENCE
#define TARGET_PASS_BY_REFERENCE tc32_pass_by_reference
#undef TARGET_ARG_PARTIAL_BYTES
#define TARGET_ARG_PARTIAL_BYTES tc32_arg_partial_bytes
#undef TARGET_ASM_FILE_START
#define TARGET_ASM_FILE_START tc32_file_start
#undef TARGET_ASM_FILE_END
#define TARGET_ASM_FILE_END tc32_file_end
#undef TARGET_ASM_FUNCTION_RODATA_SECTION
#define TARGET_ASM_FUNCTION_RODATA_SECTION tc32_function_rodata_section
#undef TARGET_ASM_FUNCTION_PROLOGUE
#define TARGET_ASM_FUNCTION_PROLOGUE thumb1_output_function_prologue
#undef TARGET_RTX_COSTS
#define TARGET_RTX_COSTS tc32_rtx_costs
#undef TARGET_ADDRESS_COST
#define TARGET_ADDRESS_COST tc32_address_cost
struct gcc_target targetm = TARGET_INITIALIZER;

static struct tc32_stack_offsets current_frame_offsets;
rtx thumb_call_via_label[14];
int thumb_code = 1;
static int thumb_call_reg_needed;
static unsigned long tc32_literal_label_num;
static unsigned long tc32_cbranch_label_num;
static unsigned long tc32_indirect_call_label_num;

static const int thumb_core_reg_alloc_order[] =
{
  4, 5, 6, 7, 3, 2, 1, 0,
  14, 12, 8, 9, 10, 11, 13, 15
};

static int
tc32_bit_count (unsigned long value)
{
  int count;

  for (count = 0; value != 0; value &= value - 1)
    count++;

  return count;
}

void
tc32_override_options (void)
{
  /* Keep the late if-conversion pass disabled for now.  On large tc32
     functions such as gp_proxy.c it still interacts badly with our jump
     lowering and can reintroduce invalid control-flow shapes.  */

  /* The current tc32 jump lowering interacts pathologically with the
     postreload if-conversion pass on large functions such as gp_proxy.c.
     Keep early if-conversion enabled, but disable the late pass.  */
  flag_if_conversion2 = 0;

  /* IRA spill-slot sharing currently crashes on some large tc32 functions
     when allocno information goes missing during reload.  Use dedicated
     spill slots until the allocator interaction is understood.  */
  flag_ira_share_spill_slots = 0;

  /* tree-switch-conversion materializes CSWTCH.* table bases in pseudos that
     still survive into hard-reg-only late RTL passes on tc32.  Until tc32 can
     legally lower those symbolic pseudos after reload, keep this transform
     disabled.  */
  flag_tree_switch_conversion = 0;

  /* Late hard-register copy propagation can also resurrect pseudo values
     after reload on tc32, which then reach final through plain movsi
     patterns in large SDK loops such as gp_proxyTab.c.  Keep that pass
     disabled until tc32 can reliably rematerialize or reload such values.  */
  flag_cprop_registers = 0;

  /* The remaining tc32 casesi/tablejump lowering is still not trusted on real
     hardware for large SDK switches in zcl.c.  Prefer linearized branches over
     indirect jump tables until tc32 jump-table dispatch is validated end-to-end.  */
  flag_jump_tables = 0;
}

static section *
tc32_function_rodata_section (tree decl ATTRIBUTE_UNUSED)
{
  return current_function_section ();
}

static int
number_of_first_bit_set (unsigned mask)
{
  int bit;

  for (bit = 0; (mask & (1U << bit)) == 0; ++bit)
    continue;

  return bit;
}

static bool
tc32_has_call_insn (void)
{
  rtx insn;

  for (insn = get_insns (); insn; insn = NEXT_INSN (insn))
    if (CALL_P (insn))
      return true;

  return false;
}

static bool
tc32_simple_leaf_function_p (void)
{
  int regno;

  if (tc32_has_call_insn ()
      || get_frame_size () != 0
      || crtl->outgoing_args_size != 0
      || crtl->args.pretend_args_size != 0
      || crtl->calls_eh_return)
    return false;

  for (regno = 0; regno < TC32_IP_REGNUM; regno++)
    if (df_regs_ever_live_p (regno) && !call_used_regs[regno])
      return false;

  return true;
}

static bool
tc32_has_incoming_stack_args_p (void)
{
  return (crtl->args.pretend_args_size != 0
          || (crtl->args.size >= 0
              && crtl->args.size > ((LAST_ARG_REGNUM + 1) * UNITS_PER_WORD)));
}

static bool
tc32_promote_prototypes (const_tree t ATTRIBUTE_UNUSED)
{
  return true;
}

static bool
tc32_frame_pointer_required (void)
{
  return (tc32_has_incoming_stack_args_p ()
          || crtl->calls_eh_return);
}

static bool
tc32_real_frame_pointer_needed (void)
{
  return (frame_pointer_needed
          && (tc32_has_incoming_stack_args_p ()
              || crtl->calls_eh_return));
}

static bool
tc32_hard_frame_pointer_available_p (void)
{
  if (!tc32_real_frame_pointer_needed ())
    return false;

  return (thumb1_compute_save_reg_mask () & (1UL << HARD_FRAME_POINTER_REGNUM)) != 0;
}

static int
tc32_frame_access_base_regno (void)
{
  return (tc32_hard_frame_pointer_available_p ()
          ? HARD_FRAME_POINTER_REGNUM
          : STACK_POINTER_REGNUM);
}

static enum machine_mode
tc32_promote_function_mode (const_tree type ATTRIBUTE_UNUSED,
                            enum machine_mode mode,
                            int *punsignedp ATTRIBUTE_UNUSED,
                            const_tree fntype ATTRIBUTE_UNUSED,
                            int for_return ATTRIBUTE_UNUSED)
{
  if (GET_MODE_CLASS (mode) == MODE_INT
      && GET_MODE_SIZE (mode) < UNITS_PER_WORD)
    return SImode;

  return mode;
}

static bool
tc32_needs_doubleword_align (enum machine_mode mode, tree type)
{
  return (GET_MODE_ALIGNMENT (mode) > PARM_BOUNDARY
          || (type && TYPE_ALIGN (type) > PARM_BOUNDARY));
}

void
tc32_optimization_options (int level, int size ATTRIBUTE_UNUSED)
{
  if (level > 0)
    flag_section_anchors = 2;
}

void
tc32_order_regs_for_local_alloc (void)
{
  const int tc32_reg_alloc_order[] = REG_ALLOC_ORDER;

  memcpy (reg_alloc_order, tc32_reg_alloc_order, sizeof (reg_alloc_order));
  memcpy (reg_alloc_order, thumb_core_reg_alloc_order,
          sizeof (thumb_core_reg_alloc_order));
}

rtx
tc32_function_arg (CUMULATIVE_ARGS cum, int mode_arg, tree type,
                   int named)
{
  enum machine_mode mode = (enum machine_mode) mode_arg;
  int words;

  if (mode == VOIDmode)
    return const0_rtx;

  if ((cum.nregs & 1)
      && tc32_needs_doubleword_align (mode, type))
    cum.nregs++;

  if (cum.can_split)
    words = 1;
  else
    words = tc32_arg_words (mode, type);

  if (!named || cum.nregs + words > 4)
    return NULL_RTX;

  return gen_rtx_REG (mode, TC32_R0_REGNUM + cum.nregs);
}

void
tc32_function_arg_advance (CUMULATIVE_ARGS *cum, int mode_arg, tree type,
                           int named ATTRIBUTE_UNUSED)
{
  enum machine_mode mode = (enum machine_mode) mode_arg;
  int nregs;
  int words;

  if ((cum->nregs & 1)
      && tc32_needs_doubleword_align (mode, type))
    cum->nregs++;

  nregs = cum->nregs;
  words = tc32_arg_words (mode, type);
  cum->nregs += words;

  if (cum->nregs > 4)
    {
      cum->nregs = 4;
      if (nregs < 4)
        cum->can_split = 0;
    }
}

rtx
tc32_function_value (const_tree valtype, const_tree fn_decl_or_type ATTRIBUTE_UNUSED)
{
  enum machine_mode mode = TYPE_MODE (valtype);
  int unsignedp ATTRIBUTE_UNUSED;

  if (INTEGRAL_TYPE_P (valtype))
    mode = tc32_promote_function_mode (valtype, mode, &unsignedp,
                                       fn_decl_or_type, 1);

  return gen_rtx_REG (mode, TC32_R0_REGNUM);
}

int
tc32_return_in_memory (tree type)
{
  return int_size_in_bytes (type) > 8;
}

static bool
tc32_pass_by_reference (CUMULATIVE_ARGS *cum ATTRIBUTE_UNUSED,
                        enum machine_mode mode ATTRIBUTE_UNUSED,
                        const_tree type, bool named ATTRIBUTE_UNUSED)
{
  return type && TREE_CODE (TYPE_SIZE (type)) != INTEGER_CST;
}

static int
tc32_arg_partial_bytes (CUMULATIVE_ARGS *cum, enum machine_mode mode,
                        tree type, bool named)
{
  int nregs = cum->nregs;
  int words;

  if (!named)
    return 0;

  if ((nregs & 1)
      && tc32_needs_doubleword_align (mode, type))
    nregs++;

  words = tc32_arg_words (mode, type);

  if (4 > nregs
      && 4 < nregs + words
      && cum->can_split)
    return (4 - nregs) * UNITS_PER_WORD;

  return 0;
}

static int
tc32_arg_words (enum machine_mode mode, tree type)
{
  int size;

  if (mode == VOIDmode)
    return 0;

  if (mode == BLKmode && type)
    size = int_size_in_bytes (type);
  else
    size = GET_MODE_SIZE (mode);

  if (size < 0)
    size = UNITS_PER_WORD;

  return (size + UNITS_PER_WORD - 1) / UNITS_PER_WORD;
}

void
tc32_init_expanders (void)
{
  if (cfun)
    mark_reg_pointer (arg_pointer_rtx, PARM_BOUNDARY);
}

static bool
tc32_legitimate_stack_mem_offset_p (enum machine_mode mode, int regno,
                                    HOST_WIDE_INT offset)
{
  if (regno == STACK_POINTER_REGNUM
      && GET_MODE_SIZE (mode) >= 4)
    return (offset >= 0
            && offset + GET_MODE_SIZE (mode) <= 1024
            && (offset & 3) == 0);

  return thumb_legitimate_offset_p (mode, offset);
}

static int
tc32_resolve_arg_pointer_offset (enum machine_mode mode, HOST_WIDE_INT addend,
                                 HOST_WIDE_INT *offset)
{
  HOST_WIDE_INT sp_offset;

  sp_offset = tc32_initial_elimination_offset (ARG_POINTER_REGNUM,
                                               STACK_POINTER_REGNUM) + addend;

  if (!tc32_hard_frame_pointer_available_p ())
    {
      *offset = sp_offset;
      return STACK_POINTER_REGNUM;
    }

  {
    HOST_WIDE_INT fp_offset;

    fp_offset = tc32_initial_elimination_offset (ARG_POINTER_REGNUM,
                                                 HARD_FRAME_POINTER_REGNUM)
                + addend;

    if (tc32_legitimate_stack_mem_offset_p (mode, HARD_FRAME_POINTER_REGNUM,
                                            fp_offset))
      {
        *offset = fp_offset;
        return HARD_FRAME_POINTER_REGNUM;
      }

    if (tc32_legitimate_stack_mem_offset_p (mode, STACK_POINTER_REGNUM,
                                            sp_offset))
      {
        *offset = sp_offset;
        return STACK_POINTER_REGNUM;
      }

  *offset = fp_offset;
  return HARD_FRAME_POINTER_REGNUM;
  }
}

static int
tc32_resolve_special_base_offset (enum machine_mode mode, int regno,
                                  HOST_WIDE_INT addend, HOST_WIDE_INT *offset)
{
  if (regno == ARG_POINTER_REGNUM)
    return tc32_resolve_arg_pointer_offset (mode, addend, offset);

  if (regno == FRAME_POINTER_REGNUM)
    {
      int base_regno = tc32_frame_access_base_regno ();

      *offset = tc32_initial_elimination_offset (FRAME_POINTER_REGNUM,
                                                 base_regno) + addend;
      return base_regno;
    }

  *offset = addend;
  return regno;
}

static int
tc32_resolve_stack_base_offset (enum machine_mode mode, int regno,
                                HOST_WIDE_INT addend, HOST_WIDE_INT *offset)
{
  if (regno == ARG_POINTER_REGNUM || regno == FRAME_POINTER_REGNUM)
    return tc32_resolve_special_base_offset (mode, regno, addend, offset);

  if (regno == HARD_FRAME_POINTER_REGNUM
      && !tc32_hard_frame_pointer_available_p ())
    {
      *offset = addend;
      return STACK_POINTER_REGNUM;
    }

  *offset = addend;
  return regno;
}

HOST_WIDE_INT
tc32_initial_elimination_offset (int from, int to)
{
  struct tc32_stack_offsets *offsets;

  offsets = thumb_get_frame_offsets ();

  switch (from)
    {
    case ARG_POINTER_REGNUM:
      switch (to)
        {
        case STACK_POINTER_REGNUM:
          return offsets->outgoing_args - offsets->saved_args;

        case FRAME_POINTER_REGNUM:
          return offsets->soft_frame - offsets->saved_args;

        case HARD_FRAME_POINTER_REGNUM:
          return offsets->locals_base - offsets->saved_args;

        default:
          gcc_unreachable ();
        }
      break;

    case FRAME_POINTER_REGNUM:
      switch (to)
        {
        case STACK_POINTER_REGNUM:
          return offsets->outgoing_args - offsets->soft_frame;

        case HARD_FRAME_POINTER_REGNUM:
          return offsets->locals_base - offsets->soft_frame;

        default:
          gcc_unreachable ();
        }
      break;

    default:
      gcc_unreachable ();
    }
}

HOST_WIDE_INT
thumb_compute_initial_elimination_offset (unsigned int from, unsigned int to)
{
  return tc32_initial_elimination_offset ((int) from, (int) to);
}

int
tc32_reg_ok_for_base_p (rtx x, int strict)
{
  if (!REG_P (x))
    return 0;

  if (strict)
    return REGNO_OK_FOR_BASE_P (REGNO (x));

  return (REGNO (x) <= LAST_LO_REGNUM
          || REGNO (x) == STACK_POINTER_REGNUM
          || REGNO (x) >= FIRST_PSEUDO_REGISTER
          || x == hard_frame_pointer_rtx
          || x == arg_pointer_rtx);
}

bool
tc32_legitimate_address_p (int mode_arg, rtx x, int strict)
{
  return thumb1_legitimate_address_p ((enum machine_mode) mode_arg, x, strict);
}

static rtx
tc32_canonicalize_print_address (rtx x)
{
  if (GET_CODE (x) == REG)
    {
      HOST_WIDE_INT offset;
      int regno = REGNO (x);
      int base_regno = tc32_resolve_stack_base_offset (SImode, regno, 0,
                                                       &offset);
      rtx base = gen_rtx_REG (SImode, base_regno);

      if (base_regno == regno && offset == 0)
        return x;

      return offset ? plus_constant (base, offset) : base;
    }

  if (GET_CODE (x) == PLUS
      && REG_P (XEXP (x, 0))
      && GET_CODE (XEXP (x, 1)) == CONST_INT)
    {
      HOST_WIDE_INT total;
      int regno = REGNO (XEXP (x, 0));
      int addend = INTVAL (XEXP (x, 1));
      int base_regno = tc32_resolve_stack_base_offset (SImode, regno, addend,
                                                       &total);
      rtx base = gen_rtx_REG (SImode, base_regno);

      if (base_regno == regno && total == addend)
        return x;

      return plus_constant (base, total);
    }

  return x;
}

void
tc32_print_operand (FILE *stream, rtx x, int code ATTRIBUTE_UNUSED)
{
  if (code == 'H' || code == 'R')
    {
      if (GET_CODE (x) == REG)
        {
          fputs (reg_names[REGNO (x) + 1], stream);
          return;
        }
    }

  if (code == 'Q')
    {
      if (GET_CODE (x) == REG)
        {
          fputs (reg_names[REGNO (x)], stream);
          return;
        }
    }

  switch (GET_CODE (x))
    {
    case REG:
      fputs (reg_names[REGNO (x)], stream);
      return;

    case MEM:
      tc32_print_operand_address (stream, XEXP (x, 0));
      return;

    case CONST_INT:
      fprintf (stream, "#%ld", (long) INTVAL (x));
      return;

    default:
      output_addr_const (stream, x);
      return;
    }
}

void
tc32_print_operand_address (FILE *stream, rtx x)
{
  x = tc32_canonicalize_print_address (x);

  if (GET_CODE (x) == REG)
    fprintf (stream, "[%s]", reg_names[REGNO (x)]);
  else if (GET_CODE (x) == PLUS
           && REG_P (XEXP (x, 0))
           && GET_CODE (XEXP (x, 1)) == CONST_INT)
    {
      int regno = REGNO (XEXP (x, 0));
      HOST_WIDE_INT offset = INTVAL (XEXP (x, 1));

      /* Convert large frame-relative accesses to SP-relative form when
         that yields a directly encodable offset.  */
      if (regno == FRAME_POINTER_REGNUM
          && offset > 124
          && offset >= 0
          && (offset & 3) == 0)
        {
          struct tc32_stack_offsets *offsets = thumb_get_frame_offsets ();
          HOST_WIDE_INT sp_offset;

          sp_offset = offset + (offsets->outgoing_args - offsets->soft_frame);
          if (sp_offset >= 0
              && sp_offset <= 1020
              && (sp_offset & 3) == 0)
            {
              fprintf (stream, "[%s, #%ld]",
                       reg_names[STACK_POINTER_REGNUM],
                       (long) sp_offset);
              return;
            }
        }

      fprintf (stream, "[%s, #%ld]",
               reg_names[regno],
               (long) offset);
    }
  else if (GET_CODE (x) == PLUS
           && REG_P (XEXP (x, 0))
           && REG_P (XEXP (x, 1)))
    fprintf (stream, "[%s, %s]",
             reg_names[REGNO (XEXP (x, 0))],
             reg_names[REGNO (XEXP (x, 1))]);
  else
    output_addr_const (stream, x);
}

static bool
tc32_return_in_memory_1 (const_tree type, const_tree fntype ATTRIBUTE_UNUSED)
{
  return tc32_return_in_memory ((tree) type);
}

static rtx
tc32_function_value_1 (const_tree valtype, const_tree fn_decl_or_type,
                       bool outgoing ATTRIBUTE_UNUSED)
{
  return tc32_function_value (valtype, fn_decl_or_type);
}

static bool
tc32_can_eliminate_1 (const int from, const int to)
{
  return ((to == FRAME_POINTER_REGNUM && from == ARG_POINTER_REGNUM) ? false
          : (to == STACK_POINTER_REGNUM && frame_pointer_needed) ? false
          : (to == HARD_FRAME_POINTER_REGNUM
             && (from == ARG_POINTER_REGNUM || from == FRAME_POINTER_REGNUM)
             && !tc32_real_frame_pointer_needed ()) ? false
          : true);
}

static bool
tc32_legitimate_address_p_1 (enum machine_mode mode, rtx x, bool strict)
{
  return tc32_legitimate_address_p (mode, x, strict);
}

static rtx
tc32_legitimize_address_1 (rtx x, rtx oldx,
                           enum machine_mode mode)
{
  return thumb_legitimize_address (x, oldx, mode);
}

static void
tc32_file_start (void)
{
  fprintf (asm_out_file, "\t.code\t16\n");
}

static void
tc32_file_end (void)
{
  int regno;

  if (!thumb_call_reg_needed)
    return;

  switch_to_section (text_section);
  ASM_OUTPUT_ALIGN (asm_out_file, 1);

  for (regno = 0; regno < RETURN_ADDRESS_POINTER_REGNUM; regno++)
    {
      rtx label = thumb_call_via_label[regno];

      if (label != 0)
        {
          targetm.asm_out.internal_label (asm_out_file, "L",
                                          CODE_LABEL_NUMBER (label));
          fprintf (asm_out_file, "\ttjex\t%s\n", reg_names[regno]);
        }
    }
}

static inline int
thumb1_rtx_costs (rtx x, enum rtx_code code, enum rtx_code outer)
{
  switch (code)
    {
    case ASHIFT:
    case ASHIFTRT:
    case LSHIFTRT:
    case ROTATERT:
    case PLUS:
    case MINUS:
    case COMPARE:
    case NEG:
    case NOT:
      return COSTS_N_INSNS (1);

    case MULT:
      if (GET_CODE (XEXP (x, 1)) == CONST_INT)
        {
          int cycles = 0;
          unsigned HOST_WIDE_INT i = INTVAL (XEXP (x, 1));

          while (i)
            {
              i >>= 2;
              cycles++;
            }

          return COSTS_N_INSNS (2) + cycles;
        }

      return COSTS_N_INSNS (1) + 16;

    case SET:
      return (COSTS_N_INSNS (1)
              + 4 * ((GET_CODE (SET_SRC (x)) == MEM)
                     + (GET_CODE (SET_DEST (x)) == MEM)));

    case CONST_INT:
      if (outer == SET)
        {
          if ((unsigned HOST_WIDE_INT) INTVAL (x) < 256)
            return 0;
          if (thumb_shiftable_const (INTVAL (x)))
            return COSTS_N_INSNS (2);
          return COSTS_N_INSNS (3);
        }
      else if ((outer == PLUS || outer == COMPARE)
               && INTVAL (x) < 256 && INTVAL (x) > -256)
        return 0;
      else if ((outer == IOR || outer == XOR || outer == AND)
               && INTVAL (x) < 256 && INTVAL (x) >= -256)
        return COSTS_N_INSNS (1);
      else if (outer == AND)
        {
          int i;

          for (i = 9; i <= 31; i++)
            if ((((HOST_WIDE_INT) 1) << i) - 1 == INTVAL (x)
                || (((HOST_WIDE_INT) 1) << i) - 1 == ~INTVAL (x))
              return COSTS_N_INSNS (2);
        }
      else if (outer == ASHIFT || outer == ASHIFTRT || outer == LSHIFTRT)
        return 0;

      return COSTS_N_INSNS (2);

    case CONST:
    case CONST_DOUBLE:
    case LABEL_REF:
    case SYMBOL_REF:
      return COSTS_N_INSNS (3);

    case UDIV:
    case UMOD:
    case DIV:
    case MOD:
      return 100;

    case MEM:
      if (REG_P (XEXP (x, 0)))
        return COSTS_N_INSNS (1);
      return COSTS_N_INSNS ((GET_MODE_SIZE (GET_MODE (x)) + UNITS_PER_WORD - 1)
                            / UNITS_PER_WORD);

    default:
      return COSTS_N_INSNS (4);
    }
}

static bool
tc32_rtx_costs (rtx x, int code_arg, int outer_arg, int *total,
                bool speed ATTRIBUTE_UNUSED)
{
  enum rtx_code code = (enum rtx_code) code_arg;
  enum rtx_code outer = (enum rtx_code) outer_arg;

  *total = thumb1_rtx_costs (x, code, outer);
  return true;
}

static int
tc32_address_cost (rtx x, bool speed ATTRIBUTE_UNUSED)
{
  enum rtx_code c = GET_CODE (x);

  if (c == REG)
    return 1;
  if (c == PLUS
      && GET_CODE (XEXP (x, 0)) == REG
      && GET_CODE (XEXP (x, 1)) == CONST_INT)
    return 1;

  return 2;
}

static int
thumb_far_jump_used_p (void)
{
  rtx insn;

  for (insn = get_insns (); insn; insn = NEXT_INSN (insn))
    {
      if (GET_CODE (insn) == JUMP_INSN
          && GET_CODE (PATTERN (insn)) != ADDR_VEC
          && GET_CODE (PATTERN (insn)) != ADDR_DIFF_VEC
          && get_attr_far_jump (insn) == FAR_JUMP_YES)
        return 1;
    }

  return 0;
}

static bool
thumb_force_lr_save (void)
{
  return (tc32_has_call_insn ()
          || thumb_far_jump_used_p ()
          || df_regs_ever_live_p (RETURN_ADDRESS_POINTER_REGNUM));
}

static unsigned
tc32_size_return_regs (void)
{
  enum machine_mode mode;
  unsigned size;

  if (crtl->return_rtx != 0)
    mode = GET_MODE (crtl->return_rtx);
  else if (current_function_decl != 0)
    mode = DECL_MODE (DECL_RESULT (current_function_decl));
  else
    mode = VOIDmode;

  size = GET_MODE_SIZE (mode);
  if (mode == BLKmode)
    return 0;

  if (size > 16)
    size = 16;

  return size;
}

static void
tc32_output_chunked_adjust (FILE *f, const char *op, int dst_reg, int src_reg,
                            HOST_WIDE_INT amount)
{
  HOST_WIDE_INT remaining = amount;
  HOST_WIDE_INT max_step = 255;

  if (remaining < 0)
    {
      op = (strcmp (op, "tadd") == 0) ? "tsub" : "tadd";
      remaining = -remaining;
    }

  if (dst_reg == STACK_POINTER_REGNUM && src_reg == STACK_POINTER_REGNUM)
    max_step = 127;

  if (dst_reg != src_reg)
    fprintf (f, "\ttmov\t%s, %s\n", reg_names[dst_reg], reg_names[src_reg]);

  while (remaining > 0)
    {
      HOST_WIDE_INT step = remaining > max_step ? max_step : remaining;
      fprintf (f, "\t%s\t%s, %s, #%ld\n",
               op, reg_names[dst_reg], reg_names[dst_reg], (long) step);
      remaining -= step;
    }
}

static int
tc32_split_shiftable_const (HOST_WIDE_INT value,
                            unsigned HOST_WIDE_INT *imm_out,
                            int *shift_out)
{
  unsigned HOST_WIDE_INT val = (unsigned HOST_WIDE_INT) value & 0xffffffffu;
  int shift;

  if (!thumb_shiftable_const (val))
    return 0;

  for (shift = 0; shift < 25; shift++)
    {
      unsigned HOST_WIDE_INT mask = ((unsigned HOST_WIDE_INT) 0xff) << shift;

      if ((val & ~mask) == 0)
        {
          *imm_out = (val >> shift) & 0xff;
          *shift_out = shift;
          return 1;
        }
    }

  return 0;
}

static void
tc32_output_sp_adjust (FILE *f, HOST_WIDE_INT amount, unsigned long pushed_regs_mask)
{
  HOST_WIDE_INT remaining;

  if (amount == 0)
    return;

  if ((amount & 3) == 0 && amount >= -255 && amount <= 255)
    {
      fprintf (f, "\t%s\t%s, %s, #%ld\n",
               amount < 0 ? "tsub" : "tadd",
               reg_names[STACK_POINTER_REGNUM],
               reg_names[STACK_POINTER_REGNUM],
               (long) (amount < 0 ? -amount : amount));
      return;
    }

  if ((amount & 3) == 0)
    {
      remaining = amount;
      while (remaining != 0)
        {
          HOST_WIDE_INT step;

          if (remaining > 0)
            step = remaining > 252 ? 252 : remaining;
          else
            step = remaining < -252 ? -252 : remaining;

          fprintf (f, "\t%s\t%s, %s, #%ld\n",
                   step < 0 ? "tsub" : "tadd",
                   reg_names[STACK_POINTER_REGNUM],
                   reg_names[STACK_POINTER_REGNUM],
                   (long) (step < 0 ? -step : step));
          remaining -= step;
        }
      return;
    }

  {
    char label[64];
    int regno = thumb_find_work_register (pushed_regs_mask);

    gcc_assert (regno >= 0 && regno <= LAST_LO_REGNUM);

    sprintf (label, ".LTC32CP%lu", tc32_literal_label_num++);
    fprintf (f, "\ttloadr\t%s, %s\n", reg_names[regno], label);
    fprintf (f, "\ttadd\t%s, %s, %s\n",
             reg_names[STACK_POINTER_REGNUM],
             reg_names[STACK_POINTER_REGNUM],
             reg_names[regno]);
    fputs ("\t.align\t2\n", f);
    fprintf (f, "%s:\n\t.word\t%ld\n", label, (long) amount);
  }
}

void
thumb1_expand_prologue (void)
{
  struct tc32_stack_offsets *offsets;
  HOST_WIDE_INT amount;
  rtx insn;

  if (tc32_simple_leaf_function_p ())
    return;

  offsets = thumb_get_frame_offsets ();
  amount = offsets->outgoing_args - offsets->saved_regs;

  if (amount > 0)
    {
      if (amount <= 255)
        insn = emit_insn (gen_subsi3 (stack_pointer_rtx, stack_pointer_rtx,
                                      GEN_INT (amount)));
      else
        {
          int regno = thumb_find_work_register (offsets->saved_regs_mask);
          rtx tmp = gen_rtx_REG (SImode, regno);

          emit_insn (gen_movsi (tmp, GEN_INT (amount)));
          insn = emit_insn (gen_subsi3 (stack_pointer_rtx, stack_pointer_rtx,
                                        tmp));
        }
      RTX_FRAME_RELATED_P (insn) = 1;
    }

  if (tc32_real_frame_pointer_needed ())
    thumb_set_frame_pointer ();
}

void
thumb1_expand_epilogue (void)
{
  struct tc32_stack_offsets *offsets;
  HOST_WIDE_INT amount;
  int regno;

  if (tc32_simple_leaf_function_p ())
    return;

  offsets = thumb_get_frame_offsets ();

  if (tc32_real_frame_pointer_needed ())
    {
      emit_insn (gen_movsi (stack_pointer_rtx, hard_frame_pointer_rtx));
      amount = offsets->locals_base - offsets->saved_regs;
    }
  else
    amount = offsets->outgoing_args - offsets->saved_regs;

  gcc_assert (amount >= 0);
  if (amount > 0)
    {
      if (amount <= 255)
        emit_insn (gen_addsi3 (stack_pointer_rtx, stack_pointer_rtx,
                               GEN_INT (amount)));
      else
        {
          rtx tmp = gen_rtx_REG (SImode, LAST_ARG_REGNUM);

          emit_insn (gen_movsi (tmp, GEN_INT (amount)));
          emit_insn (gen_addsi3 (stack_pointer_rtx, stack_pointer_rtx, tmp));
        }
    }

  emit_insn (gen_prologue_use (stack_pointer_rtx));

  for (regno = 0; regno < TC32_IP_REGNUM; regno++)
    if (df_regs_ever_live_p (regno) && !call_used_regs[regno])
      emit_clobber (gen_rtx_REG (SImode, regno));

  if (!df_regs_ever_live_p (TC32_LR_REGNUM))
    emit_use (gen_rtx_REG (SImode, TC32_LR_REGNUM));
}

static void
thumb_pushpop (FILE *f, unsigned long mask, int push,
               int *cfa_offset ATTRIBUTE_UNUSED, unsigned long real_regs ATTRIBUTE_UNUSED)
{
  int regno;
  int first;
  int last_regno;
  const char *mnemonic;

  gcc_assert (mask);

  mnemonic = push ? "tpush" : "tpop";
  last_regno = push ? TC32_LR_REGNUM : TC32_PC_REGNUM;
  fprintf (f, "\t%s\t{", mnemonic);

  first = 1;
  for (regno = 0; regno <= last_regno; regno++)
    if (mask & (1UL << regno))
      {
        if (!first)
          fprintf (f, ", ");
        fprintf (f, "%s", reg_names[regno]);
        first = 0;
      }

  fprintf (f, "}\n");
}

static void
thumb_exit (FILE *f, int reg_containing_return_addr)
{
  if (crtl->calls_eh_return)
    fprintf (f, "\ttadd\t%s, %s\n",
             reg_names[TC32_SP_REGNUM], reg_names[TC32_EH_STACKADJ_REGNUM]);

  if (reg_containing_return_addr == -1)
    {
      fprintf (f, "\ttpop\t{%s}\n", reg_names[LAST_ARG_REGNUM]);
      reg_containing_return_addr = LAST_ARG_REGNUM;
    }

  fprintf (f, "\ttjex\t%s\n", reg_names[reg_containing_return_addr]);
}

static struct tc32_stack_offsets *
thumb_get_frame_offsets (void)
{
  HOST_WIDE_INT frame_size;
  int leaf;

  frame_size = (get_frame_size () + 3) & ~3;
  leaf = leaf_function_p ();

  if (tc32_simple_leaf_function_p ())
    {
      current_frame_offsets.saved_args = 0;
      current_frame_offsets.saved_regs_mask = 0;
      current_frame_offsets.saved_regs = 0;
      current_frame_offsets.soft_frame = 0;
      current_frame_offsets.locals_base = 0;
      current_frame_offsets.outgoing_args = 0;
      return &current_frame_offsets;
    }

  current_frame_offsets.saved_args = crtl->args.pretend_args_size;
  current_frame_offsets.saved_regs_mask = thumb1_compute_save_reg_mask ();
  current_frame_offsets.saved_regs = (current_frame_offsets.saved_args
                                      + tc32_bit_count (current_frame_offsets.saved_regs_mask)
                                      * UNITS_PER_WORD);
  current_frame_offsets.soft_frame = current_frame_offsets.saved_regs;

  if (leaf && frame_size == 0)
    {
      current_frame_offsets.locals_base = current_frame_offsets.soft_frame;
      current_frame_offsets.outgoing_args = current_frame_offsets.soft_frame;
      return &current_frame_offsets;
    }

  current_frame_offsets.locals_base = current_frame_offsets.soft_frame + frame_size;
  current_frame_offsets.outgoing_args =
    current_frame_offsets.locals_base + crtl->outgoing_args_size;

  return &current_frame_offsets;
}

static int
thumb_find_work_register (unsigned long pushed_regs_mask)
{
  int reg;

  for (reg = LAST_ARG_REGNUM; reg >= 0; reg--)
    if (!df_regs_ever_live_p (reg))
      return reg;

  for (reg = LAST_LO_REGNUM; reg > LAST_ARG_REGNUM; reg--)
    if (pushed_regs_mask & (1UL << reg))
      return reg;

  return LAST_LO_REGNUM;
}

static unsigned long
thumb1_compute_save_reg_mask (void)
{
  unsigned long mask;
  unsigned reg;

  mask = 0;
  for (reg = 0; reg < 12; reg++)
    if (df_regs_ever_live_p (reg)
        && !call_used_regs[reg]
        && (reg != HARD_FRAME_POINTER_REGNUM || tc32_real_frame_pointer_needed ()))
      mask |= 1UL << reg;

  if (tc32_real_frame_pointer_needed ())
    mask |= 1UL << HARD_FRAME_POINTER_REGNUM;

  if ((mask & 0xff) || thumb_force_lr_save ())
    mask |= 1UL << RETURN_ADDRESS_POINTER_REGNUM;

  if ((((get_frame_size () + UNITS_PER_WORD - 1) & -UNITS_PER_WORD)
       + crtl->outgoing_args_size) >= 504)
    {
      for (reg = LAST_ARG_REGNUM + 1; reg <= LAST_LO_REGNUM; reg++)
        if (mask & (1UL << reg))
          break;

      if (reg > LAST_LO_REGNUM)
        {
          reg = thumb_find_work_register (1UL << LAST_LO_REGNUM);
          /* Mirror Thumb1 behavior: do not reserve a return-value register
             as the forced low work register for large stack adjustments.  */
          if ((unsigned) reg * UNITS_PER_WORD < tc32_size_return_regs ())
            reg = LAST_LO_REGNUM;
          if (!call_used_regs[reg])
            mask |= 1UL << reg;
        }
    }

  return mask;
}
/* Copied from GCC ARM Thumb1 path intentionally with original names kept.  */
static int
thumb1_base_register_rtx_p (rtx x, enum machine_mode mode, int strict_p)
{
  int regno;

  if (GET_CODE (x) != REG)
    return 0;

  regno = REGNO (x);

  if (strict_p)
    return THUMB1_REGNO_MODE_OK_FOR_BASE_P (regno, mode);

  return (regno <= LAST_LO_REGNUM
          || regno > LAST_VIRTUAL_REGISTER
          || regno == FRAME_POINTER_REGNUM
          || (GET_MODE_SIZE (mode) >= 4
              && (regno == STACK_POINTER_REGNUM
                  || regno >= FIRST_PSEUDO_REGISTER
                  || x == hard_frame_pointer_rtx
                  || x == arg_pointer_rtx)));
}

inline static int
thumb1_index_register_rtx_p (rtx x, int strict_p)
{
  return thumb1_base_register_rtx_p (x, QImode, strict_p);
}

static int
thumb1_legitimate_address_p (enum machine_mode mode, rtx x, int strict_p)
{
  if (GET_MODE_SIZE (mode) < 4
      && !(reload_in_progress || reload_completed)
      && (reg_mentioned_p (frame_pointer_rtx, x)
          || reg_mentioned_p (arg_pointer_rtx, x)
          || reg_mentioned_p (virtual_incoming_args_rtx, x)
          || reg_mentioned_p (virtual_outgoing_args_rtx, x)
          || reg_mentioned_p (virtual_stack_dynamic_rtx, x)
          || reg_mentioned_p (virtual_stack_vars_rtx, x)))
    return 0;

  else if (thumb1_base_register_rtx_p (x, mode, strict_p))
    return 1;

  else if (GET_MODE_SIZE (mode) >= 4 && CONSTANT_P (x)
           && GET_CODE (x) == SYMBOL_REF
           && CONSTANT_POOL_ADDRESS_P (x) && !flag_pic)
    return 1;

  else if (GET_MODE_SIZE (mode) >= 4
           && reload_completed
           && (GET_CODE (x) == LABEL_REF
               || (GET_CODE (x) == CONST
                   && GET_CODE (XEXP (x, 0)) == PLUS
                   && GET_CODE (XEXP (XEXP (x, 0), 0)) == LABEL_REF
                   && GET_CODE (XEXP (XEXP (x, 0), 1)) == CONST_INT)))
    return 1;

  else if (GET_CODE (x) == POST_INC && GET_MODE_SIZE (mode) >= 4
           && thumb1_index_register_rtx_p (XEXP (x, 0), strict_p))
    return 1;

  else if (GET_CODE (x) == PLUS)
    {
      if (thumb1_index_register_rtx_p (XEXP (x, 0), strict_p)
          && thumb1_index_register_rtx_p (XEXP (x, 1), strict_p))
        return 1;

      if ((thumb1_index_register_rtx_p (XEXP (x, 0), strict_p)
                || XEXP (x, 0) == arg_pointer_rtx)
               && GET_CODE (XEXP (x, 1)) == CONST_INT
               && thumb_legitimate_offset_p (mode, INTVAL (XEXP (x, 1))))
        return 1;

      else if (GET_CODE (XEXP (x, 0)) == REG
               && REGNO (XEXP (x, 0)) == STACK_POINTER_REGNUM
               && GET_MODE_SIZE (mode) >= 4
               && GET_CODE (XEXP (x, 1)) == CONST_INT
               && INTVAL (XEXP (x, 1)) >= 0
               && INTVAL (XEXP (x, 1)) + GET_MODE_SIZE (mode) <= 1024
               && (INTVAL (XEXP (x, 1)) & 3) == 0)
        return 1;

      else if (GET_CODE (XEXP (x, 0)) == REG
               && (REGNO (XEXP (x, 0)) == FRAME_POINTER_REGNUM
                   || REGNO (XEXP (x, 0)) == ARG_POINTER_REGNUM
                   || (REGNO (XEXP (x, 0)) >= FIRST_VIRTUAL_REGISTER
                       && REGNO (XEXP (x, 0)) <= LAST_VIRTUAL_REGISTER))
               && GET_MODE_SIZE (mode) >= 4
               && GET_CODE (XEXP (x, 1)) == CONST_INT
               && thumb_legitimate_offset_p (mode, INTVAL (XEXP (x, 1))))
        return 1;
    }

  else if (GET_MODE_CLASS (mode) != MODE_FLOAT
           && GET_MODE_SIZE (mode) == 4
           && GET_CODE (x) == SYMBOL_REF
           && CONSTANT_POOL_ADDRESS_P (x)
           && !flag_pic)
    return 1;

  return 0;
}

static int
thumb_legitimate_offset_p (enum machine_mode mode, HOST_WIDE_INT val)
{
  switch (GET_MODE_SIZE (mode))
    {
    case 1:
      return val >= 0 && val < 32;

    case 2:
      return val >= 0 && val < 64 && (val & 1) == 0;

    default:
      return (val >= 0
              && (val + GET_MODE_SIZE (mode)) <= 128
              && (val & 3) == 0);
    }
}

static rtx
thumb_legitimize_address (rtx x, rtx orig_x ATTRIBUTE_UNUSED,
                          enum machine_mode mode)
{
  if (x == virtual_incoming_args_rtx)
    return arg_pointer_rtx;

  if (GET_CODE (x) == PLUS
      && XEXP (x, 0) == virtual_incoming_args_rtx
      && GET_CODE (XEXP (x, 1)) == CONST_INT)
    return plus_constant (arg_pointer_rtx, INTVAL (XEXP (x, 1)));

  if (GET_CODE (x) == PLUS
      && REG_P (XEXP (x, 0))
      && REG_P (XEXP (x, 1)))
    return force_operand (x, NULL_RTX);

  if (GET_CODE (x) == PLUS
      && !s_register_operand (XEXP (x, 0), SImode)
      && GET_CODE (XEXP (x, 1)) == CONST_INT)
    {
      rtx xop0 = force_operand (XEXP (x, 0), NULL_RTX);
      HOST_WIDE_INT offset = INTVAL (XEXP (x, 1));

      if (thumb_legitimate_offset_p (mode, offset))
        x = plus_constant (xop0, offset);
      else
        {
          rtx xop1 = force_reg (SImode, XEXP (x, 1));
          x = gen_rtx_PLUS (SImode, xop0, xop1);
        }

      return x;
    }

  if (GET_CODE (x) == PLUS
      && GET_CODE (XEXP (x, 1)) == CONST_INT
      && (INTVAL (XEXP (x, 1)) >= 32 * GET_MODE_SIZE (mode)
          || INTVAL (XEXP (x, 1)) < 0))
    {
      rtx xop0 = XEXP (x, 0);
      rtx xop1 = XEXP (x, 1);
      HOST_WIDE_INT offset = INTVAL (xop1);

      if (optimize_size && offset >= 0
          && offset < 256 + 31 * GET_MODE_SIZE (mode))
        {
          HOST_WIDE_INT delta;

          if (offset >= 256)
            delta = offset - (256 - GET_MODE_SIZE (mode));
          else if (offset < 32 * GET_MODE_SIZE (mode) + 8)
            delta = 31 * GET_MODE_SIZE (mode);
          else
            delta = offset & (~(31 * GET_MODE_SIZE (mode)));

          xop0 = force_operand (plus_constant (xop0, offset - delta),
                                NULL_RTX);
          x = plus_constant (xop0, delta);
        }
      else if (offset < 0 && offset > -256)
        x = force_operand (x, NULL_RTX);
      else
        {
          xop1 = force_reg (SImode, xop1);
          x = gen_rtx_PLUS (SImode, xop0, xop1);
        }
    }
  else if (GET_CODE (x) == PLUS
           && s_register_operand (XEXP (x, 1), SImode)
           && !s_register_operand (XEXP (x, 0), SImode))
    {
      rtx xop0 = force_operand (XEXP (x, 0), NULL_RTX);

      x = gen_rtx_PLUS (SImode, xop0, XEXP (x, 1));
    }

  return x;
}

rtx
thumb_legitimize_reload_address (rtx *x_p, int mode_arg,
                                 int opnum, int type,
                                 int ind_levels ATTRIBUTE_UNUSED)
{
  enum machine_mode mode = (enum machine_mode) mode_arg;
  rtx x = *x_p;

  if (GET_CODE (x) == PLUS
      && GET_MODE_SIZE (mode) < 4
      && REG_P (XEXP (x, 0))
      && XEXP (x, 0) == stack_pointer_rtx
      && GET_CODE (XEXP (x, 1)) == CONST_INT
      && !thumb_legitimate_offset_p (mode, INTVAL (XEXP (x, 1))))
    {
      rtx orig_x = x;

      x = copy_rtx (x);
      push_reload (orig_x, NULL_RTX, x_p, NULL, MODE_BASE_REG_CLASS (mode),
                   Pmode, VOIDmode, 0, 0, opnum, (enum reload_type) type);
      return x;
    }

  if (GET_CODE (x) == PLUS
      && REG_P (XEXP (x, 0))
      && GET_CODE (XEXP (x, 1)) == CONST_INT
      && (INTVAL (XEXP (x, 1)) < 0
          || REGNO (XEXP (x, 0)) == STACK_POINTER_REGNUM
          || REGNO (XEXP (x, 0)) == FRAME_POINTER_REGNUM
          || REGNO (XEXP (x, 0)) == HARD_FRAME_POINTER_REGNUM
          || REGNO (XEXP (x, 0)) == ARG_POINTER_REGNUM)
      && !thumb1_legitimate_address_p (mode, x, 0))
    {
      rtx orig_x = x;

      x = copy_rtx (x);
      push_reload (orig_x, NULL_RTX, x_p, NULL, MODE_BASE_REG_CLASS (mode),
                   Pmode, VOIDmode, 0, 0, opnum, (enum reload_type) type);
      return x;
    }

  if (GET_CODE (x) == PLUS
      && REG_P (XEXP (x, 0))
      && REG_P (XEXP (x, 1))
      && !thumb1_legitimate_address_p (mode, x, 1))
    {
      rtx orig_x = x;

      x = copy_rtx (x);
      push_reload (orig_x, NULL_RTX, x_p, NULL, MODE_BASE_REG_CLASS (mode),
                   Pmode, VOIDmode, 0, 0, opnum, (enum reload_type) type);
      return x;
    }

  if (GET_CODE (x) == PLUS
      && REG_P (XEXP (x, 0))
      && REG_P (XEXP (x, 1))
      && !tc32_reg_ok_for_base_p (XEXP (x, 0), 0)
      && !tc32_reg_ok_for_base_p (XEXP (x, 1), 0))
    {
      rtx orig_x = x;

      x = copy_rtx (x);
      push_reload (orig_x, NULL_RTX, x_p, NULL, MODE_BASE_REG_CLASS (mode),
                   Pmode, VOIDmode, 0, 0, opnum, (enum reload_type) type);
      return x;
    }

  return NULL_RTX;
}

void
thumb1_final_prescan_insn (rtx insn)
{
  insn = insn;
}

int
thumb_shiftable_const (unsigned HOST_WIDE_INT val)
{
  unsigned HOST_WIDE_INT mask = 0xff;
  int i;

  val &= (unsigned HOST_WIDE_INT) 0xffffffffu;
  if (val == 0)
    return 0;

  for (i = 0; i < 25; i++)
    if ((val & (mask << i)) == val)
      return 1;

  return 0;
}

static void
tc32_emit_const_literal_load (rtx dst, rtx src)
{
  char label[64];
  char cont[64];
  rtx plus;
  rtx base;
  rtx offset;
  rtx ops[3];
  rtx low_dst = dst;

  if (tc32_constant_pool_ref_p (src))
    src = tc32_get_pool_ref_constant (src);

  if (tc32_hi_reg_p (dst))
    low_dst = tc32_choose_low_scratch (REGNO (dst), -1);

  if (GET_CODE (src) == CONST)
    {
      plus = XEXP (src, 0);

      if (GET_CODE (plus) == PLUS)
        {
          base = XEXP (plus, 0);
          offset = XEXP (plus, 1);

          if ((GET_CODE (base) == SYMBOL_REF || GET_CODE (base) == LABEL_REF)
              && GET_CODE (offset) == CONST_INT
              && INTVAL (offset) != 0
              && INTVAL (offset) >= -255
              && INTVAL (offset) <= 255)
            {
              tc32_emit_const_literal_load (low_dst, base);
              ops[0] = low_dst;
              ops[1] = low_dst;
              ops[2] = GEN_INT (INTVAL (offset) > 0
                                ? INTVAL (offset)
                                : -INTVAL (offset));

              output_asm_insn (INTVAL (offset) > 0
                               ? "tadd\t%0, %1, %2"
                               : "tsub\t%0, %1, %2",
                               ops);

              if (low_dst != dst)
                {
                  ops[0] = dst;
                  ops[1] = low_dst;
                  output_asm_insn ("tmov\t%0, %1", ops);
                }
              return;
            }
        }
    }

  sprintf (label, ".LTC32CP%lu", tc32_literal_label_num++);
  sprintf (cont, ".LTC32CL%lu", tc32_literal_label_num++);
  fprintf (asm_out_file, "\ttloadr\t%s, %s\n",
           reg_names[REGNO (low_dst)], label);
  fprintf (asm_out_file, "\ttj\t%s\n", cont);
  fputs ("\t.align\t2\n", asm_out_file);
  fprintf (asm_out_file, "%s:\n\t.word\t", label);
  output_addr_const (asm_out_file, src);
  fputc ('\n', asm_out_file);
  fprintf (asm_out_file, "%s:\n", cont);

  if (low_dst != dst)
    fprintf (asm_out_file, "\ttmov\t%s, %s\n",
             reg_names[REGNO (dst)], reg_names[REGNO (low_dst)]);
}

static bool
tc32_constant_pool_ref_p (rtx x)
{
  if (GET_CODE (x) == SYMBOL_REF)
    return CONSTANT_POOL_ADDRESS_P (x);

  if (GET_CODE (x) == CONST)
    {
      rtx plus = XEXP (x, 0);
      rtx base = XEXP (plus, 0);

      return (GET_CODE (plus) == PLUS
              && GET_CODE (base) == SYMBOL_REF
              && CONSTANT_POOL_ADDRESS_P (base)
              && GET_CODE (XEXP (plus, 1)) == CONST_INT);
    }

  return false;
}

static rtx
tc32_strip_subreg (rtx x)
{
  if (GET_CODE (x) == SUBREG)
    x = SUBREG_REG (x);

  return x;
}

static bool
tc32_same_reg_operand_p (rtx a, rtx b)
{
  a = tc32_strip_subreg (a);
  b = tc32_strip_subreg (b);

  if (REG_P (a) && REG_P (b))
    return REGNO (a) == REGNO (b);

  return rtx_equal_p (a, b);
}

static rtx
tc32_get_pool_ref_constant (rtx x)
{
  if (GET_CODE (x) == SYMBOL_REF)
    return get_pool_constant (x);

  if (GET_CODE (x) == CONST)
    {
      rtx plus = XEXP (x, 0);
      rtx base = get_pool_constant (XEXP (plus, 0));
      rtx offset = XEXP (plus, 1);

      return plus_constant (base, INTVAL (offset));
    }

  gcc_unreachable ();
}

static rtx
tc32_choose_low_scratch (int avoid_a, int avoid_b)
{
  static const int scratch_order[] = { 3, 2, 1, 0, 4, 5, 6 };
  int i;

  for (i = 0; i < (int) ARRAY_SIZE (scratch_order); ++i)
    {
      int regno = scratch_order[i];

      if (regno == avoid_a || regno == avoid_b)
        continue;

      if (find_regno_note (current_output_insn, REG_DEAD, regno))
        return gen_rtx_REG (SImode, regno);
    }

  for (i = 0; i < (int) ARRAY_SIZE (scratch_order); ++i)
    {
      int regno = scratch_order[i];

      if (regno == avoid_a || regno == avoid_b)
        continue;

      return gen_rtx_REG (SImode, regno);
    }

  gcc_unreachable ();
}

static bool
tc32_reg_dead_here_p (int regno)
{
  return find_regno_note (current_output_insn, REG_DEAD, regno) != 0;
}

static bool
tc32_hi_reg_p (rtx x)
{
  if (GET_CODE (x) == SUBREG)
    x = SUBREG_REG (x);

  return (REG_P (x)
          && REGNO (x) >= 8
          && REGNO (x) <= 11);
}

static rtx
tc32_copy_to_low_reg (rtx reg, int avoid_a, int avoid_b)
{
  rtx tmp;
  rtx ops[2];

  if (!tc32_hi_reg_p (reg))
    return reg;

  tmp = tc32_choose_low_scratch (avoid_a, avoid_b);
  ops[0] = tmp;
  ops[1] = reg;
  output_asm_insn ("tmov\t%0, %1", ops);
  return tmp;
}

static void
tc32_emit_address_to_reg (rtx target, rtx addr, int avoid)
{
  rtx ops[3];

  if (GET_CODE (addr) == SUBREG)
    addr = SUBREG_REG (addr);

  if (REG_P (addr))
    {
      ops[0] = target;
      ops[1] = addr;
      output_asm_insn ("tmov\t%0, %1", ops);
      return;
    }

  if (GET_CODE (addr) == SYMBOL_REF
      || GET_CODE (addr) == LABEL_REF
      || GET_CODE (addr) == CONST)
    {
      tc32_emit_const_literal_load (target, addr);
      return;
    }

  if (GET_CODE (addr) == MULT
      && REG_P (XEXP (addr, 0))
      && GET_CODE (XEXP (addr, 1)) == CONST_INT)
    {
      HOST_WIDE_INT scale = INTVAL (XEXP (addr, 1));
      int shift = 0;

      if (scale <= 0 || (scale & (scale - 1)) != 0)
        gcc_unreachable ();

      while ((1L << shift) != scale)
        shift++;

      ops[0] = target;
      ops[1] = XEXP (addr, 0);
      output_asm_insn ("tmov\t%0, %1", ops);
      if (shift != 0)
        {
          char buf[32];
          sprintf (buf, "tshftl\t%%0, %%0, #%d", shift);
          output_asm_insn (buf, ops);
        }
      return;
    }

  if (GET_CODE (addr) == AND)
    {
      rtx lhs = XEXP (addr, 0);
      rtx rhs = XEXP (addr, 1);
      rtx low_target = target;
      rtx saved_target = NULL_RTX;
      rtx mask = rhs;
      rtx saved_mask = NULL_RTX;

      if (tc32_hi_reg_p (target))
        {
          low_target = tc32_choose_low_scratch (REGNO (target), avoid);
          if (!tc32_reg_dead_here_p (REGNO (low_target)))
            {
              saved_target = low_target;
              ops[0] = low_target;
              output_asm_insn ("tpush\t{%0}", ops);
            }
        }

      tc32_emit_address_to_reg (low_target, lhs, avoid);

      if (GET_CODE (mask) == CONST_INT)
        {
          mask = tc32_choose_low_scratch (REGNO (low_target), avoid);
          if (!tc32_reg_dead_here_p (REGNO (mask)))
            {
              saved_mask = mask;
              ops[0] = mask;
              output_asm_insn ("tpush\t{%0}", ops);
            }

          if (INTVAL (rhs) >= 0 && INTVAL (rhs) <= 255)
            {
              ops[0] = mask;
              ops[1] = rhs;
              output_asm_insn ("tmov\t%0, %1", ops);
            }
          else
            tc32_emit_const_literal_load (mask, rhs);
        }
      else
        {
          if (tc32_hi_reg_p (mask))
            {
              mask = tc32_choose_low_scratch (REGNO (low_target), avoid);
              if (!tc32_reg_dead_here_p (REGNO (mask)))
                {
                  saved_mask = mask;
                  ops[0] = mask;
                  output_asm_insn ("tpush\t{%0}", ops);
                }

              tc32_emit_address_to_reg (mask, rhs, REGNO (low_target));
            }
        }

      ops[0] = low_target;
      ops[1] = low_target;
      ops[2] = mask;
      output_asm_insn ("tand\t%0, %1, %2", ops);

      if (saved_mask)
        {
          ops[0] = saved_mask;
          output_asm_insn ("tpop\t{%0}", ops);
        }

      if (low_target != target)
        {
          ops[0] = target;
          ops[1] = low_target;
          output_asm_insn ("tmov\t%0, %1", ops);
        }

      if (saved_target)
        {
          ops[0] = saved_target;
          output_asm_insn ("tpop\t{%0}", ops);
        }
      return;
    }

  if (GET_CODE (addr) == PLUS)
    {
      rtx lhs = XEXP (addr, 0);
      rtx rhs = XEXP (addr, 1);

      if (GET_CODE (rhs) == CONST_INT)
        {
          tc32_emit_address_to_reg (target, lhs, avoid);
          ops[0] = target;
          ops[1] = target;
          tc32_emit_chunked_addsub ("tadd", ops, INTVAL (rhs));
          return;
        }

      if (GET_CODE (lhs) == CONST_INT)
        {
          tc32_emit_address_to_reg (target, rhs, avoid);
          ops[0] = target;
          ops[1] = target;
          tc32_emit_chunked_addsub ("tadd", ops, INTVAL (lhs));
          return;
        }

      if (REG_P (lhs) && REG_P (rhs))
        {
          ops[0] = target;
          ops[1] = lhs;
          ops[2] = rhs;
          output_asm_insn ("tadd\t%0, %1, %2", ops);
          return;
        }

      {
        rtx tmp = tc32_choose_low_scratch (REGNO (target), avoid);

        ops[0] = tmp;
        output_asm_insn ("tpush\t{%0}", ops);
        tc32_emit_address_to_reg (target, lhs, REGNO (tmp));
        tc32_emit_address_to_reg (tmp, rhs, REGNO (target));
        ops[0] = target;
        ops[1] = target;
        ops[2] = tmp;
        output_asm_insn ("tadd\t%0, %1, %2", ops);
        ops[0] = tmp;
        output_asm_insn ("tpop\t{%0}", ops);
        return;
      }
    }

  gcc_unreachable ();
}

static void
tc32_emit_lowreg_load (const char *templ, rtx dst, rtx mem)
{
  rtx addr = XEXP (mem, 0);
  rtx base = NULL_RTX;
  HOST_WIDE_INT offset = 0;
  int dst_regno = REG_P (dst) ? REGNO (dst) : -1;
  rtx low_base = base;
  rtx low_dst = dst;
  rtx saved_base = NULL_RTX;
  rtx saved_dst = NULL_RTX;
  rtx ops[2];

  if (GET_CODE (addr) == REG)
    base = addr;
  else if (GET_CODE (addr) == PLUS
           && REG_P (XEXP (addr, 0))
           && GET_CODE (XEXP (addr, 1)) == CONST_INT)
    {
      base = XEXP (addr, 0);
      offset = INTVAL (XEXP (addr, 1));
    }
  else
    {
      low_base = tc32_choose_low_scratch ((dst_regno >= 0
                                           && dst_regno <= LAST_LO_REGNUM)
                                          ? dst_regno : -1,
                                          -1);
      saved_base = low_base;
      ops[0] = low_base;
      output_asm_insn ("tpush\t{%0}", ops);
      tc32_emit_address_to_reg (low_base, addr, -1);

      if (tc32_hi_reg_p (dst))
        {
          low_dst = tc32_choose_low_scratch (REGNO (low_base), -1);
          if (!tc32_reg_dead_here_p (REGNO (low_dst)))
            {
              saved_dst = low_dst;
              ops[0] = low_dst;
              output_asm_insn ("tpush\t{%0}", ops);
            }
        }

      ops[0] = low_dst;
      ops[1] = gen_rtx_MEM (GET_MODE (mem), low_base);
      output_asm_insn (templ, ops);

      if (low_dst != dst)
        {
          ops[0] = dst;
          ops[1] = low_dst;
          output_asm_insn ("tmov\t%0, %1", ops);
        }

      if (saved_dst)
        {
          ops[0] = saved_dst;
          output_asm_insn ("tpop\t{%0}", ops);
        }

      ops[0] = saved_base;
      output_asm_insn ("tpop\t{%0}", ops);
      return;
    }

  low_base = base;

  if (tc32_hi_reg_p (base))
    {
      low_base = tc32_choose_low_scratch ((dst_regno >= 0
                                           && dst_regno <= LAST_LO_REGNUM)
                                          ? dst_regno : -1,
                                          -1);
      saved_base = low_base;
      ops[0] = low_base;
      output_asm_insn ("tpush\t{%0}", ops);

      ops[0] = low_base;
      ops[1] = base;
      output_asm_insn ("tmov\t%0, %1", ops);
    }

  if (tc32_hi_reg_p (dst))
    {
      low_dst = tc32_choose_low_scratch (REGNO (low_base), -1);
      if (!tc32_reg_dead_here_p (REGNO (low_dst)))
        {
          saved_dst = low_dst;
          ops[0] = low_dst;
          output_asm_insn ("tpush\t{%0}", ops);
        }
    }

  ops[0] = low_dst;
  ops[1] = gen_rtx_MEM (GET_MODE (mem),
                        offset ? plus_constant (low_base, offset) : low_base);
  output_asm_insn (templ, ops);

  if (low_dst != dst)
    {
      ops[0] = dst;
      ops[1] = low_dst;
      output_asm_insn ("tmov\t%0, %1", ops);
    }

  if (saved_dst)
    {
      ops[0] = saved_dst;
      output_asm_insn ("tpop\t{%0}", ops);
    }

  if (saved_base)
    {
      ops[0] = saved_base;
      output_asm_insn ("tpop\t{%0}", ops);
    }
}

static void
tc32_emit_lowreg_store (const char *templ, rtx mem, rtx src)
{
  rtx addr = XEXP (mem, 0);
  rtx base = NULL_RTX;
  HOST_WIDE_INT offset = 0;
  rtx low_base = base;
  rtx low_src = src;
  rtx saved_base = NULL_RTX;
  rtx saved_src = NULL_RTX;
  rtx ops[2];
  int src_regno = REG_P (src) ? REGNO (src) : -1;

  if (GET_CODE (addr) == REG)
    base = addr;
  else if (GET_CODE (addr) == PLUS
           && REG_P (XEXP (addr, 0))
           && GET_CODE (XEXP (addr, 1)) == CONST_INT)
    {
      base = XEXP (addr, 0);
      offset = INTVAL (XEXP (addr, 1));
    }
  else
    {
      low_base = tc32_choose_low_scratch ((src_regno >= 0
                                           && src_regno <= LAST_LO_REGNUM)
                                          ? src_regno : -1,
                                          -1);
      saved_base = low_base;
      ops[0] = low_base;
      output_asm_insn ("tpush\t{%0}", ops);
      tc32_emit_address_to_reg (low_base, addr, -1);

      if (tc32_hi_reg_p (src))
        {
          low_src = tc32_choose_low_scratch (REGNO (low_base), -1);
          if (!tc32_reg_dead_here_p (REGNO (low_src)))
            {
              saved_src = low_src;
              ops[0] = low_src;
              output_asm_insn ("tpush\t{%0}", ops);
            }

          ops[0] = low_src;
          ops[1] = src;
          output_asm_insn ("tmov\t%0, %1", ops);
        }

      ops[0] = gen_rtx_MEM (GET_MODE (mem), low_base);
      ops[1] = low_src;
      output_asm_insn (templ, ops);

      if (saved_src)
        {
          ops[0] = saved_src;
          output_asm_insn ("tpop\t{%0}", ops);
        }

      ops[0] = saved_base;
      output_asm_insn ("tpop\t{%0}", ops);
      return;
    }

  if (GET_CODE (src) == SUBREG)
    src_regno = REGNO (SUBREG_REG (src));

  low_base = base;

  if (tc32_hi_reg_p (base))
    {
      low_base = tc32_choose_low_scratch ((src_regno >= 0
                                           && src_regno <= LAST_LO_REGNUM)
                                          ? src_regno : -1,
                                          -1);
      saved_base = low_base;
      ops[0] = low_base;
      output_asm_insn ("tpush\t{%0}", ops);

      ops[0] = low_base;
      ops[1] = base;
      output_asm_insn ("tmov\t%0, %1", ops);
    }

  if (tc32_hi_reg_p (src))
    {
      low_src = tc32_choose_low_scratch (REGNO (low_base), -1);
      if (!tc32_reg_dead_here_p (REGNO (low_src)))
        {
          saved_src = low_src;
          ops[0] = low_src;
          output_asm_insn ("tpush\t{%0}", ops);
        }

      ops[0] = low_src;
      ops[1] = src;
      output_asm_insn ("tmov\t%0, %1", ops);
    }

  ops[0] = gen_rtx_MEM (GET_MODE (mem),
                        offset ? plus_constant (low_base, offset) : low_base);
  ops[1] = low_src;
  output_asm_insn (templ, ops);

  if (saved_src)
    {
      ops[0] = saved_src;
      output_asm_insn ("tpop\t{%0}", ops);
    }

  if (saved_base)
    {
      ops[0] = saved_base;
      output_asm_insn ("tpop\t{%0}", ops);
    }
}

static void
tc32_emit_chunked_addsub (const char *op, rtx *operands, HOST_WIDE_INT amount)
{
  HOST_WIDE_INT remaining = amount;
  rtx ops[3];

  ops[0] = operands[0];
  ops[1] = operands[1];

  if (remaining < 0)
    {
      op = (strcmp (op, "tadd") == 0) ? "tsub" : "tadd";
      remaining = -remaining;
    }

  if (!rtx_equal_p (ops[0], ops[1]))
    output_asm_insn ("tmov\t%0, %1", ops);

  while (remaining > 0)
    {
      ops[2] = GEN_INT (remaining > 255 ? 255 : remaining);
      if (strcmp (op, "tadd") == 0)
        output_asm_insn ("tadd\t%0, %0, %2", ops);
      else
        output_asm_insn ("tsub\t%0, %0, %2", ops);
      remaining -= INTVAL (ops[2]);
    }
}

static int
tc32_emit_shifted_byte_addsub (const char *op, rtx *operands, HOST_WIDE_INT amount)
{
  HOST_WIDE_INT scaled;
  rtx tmp;
  rtx imm_ops[3];
  rtx alu_ops[3];
  rtx dst = tc32_strip_subreg (operands[0]);
  rtx src = tc32_strip_subreg (operands[1]);
  int dst_regno = REG_P (dst) ? REGNO (dst) : -1;
  int src_regno = REG_P (src) ? REGNO (src) : -1;

  if (amount == 0)
    return 0;

  if (amount < 0)
    {
      op = (strcmp (op, "tadd") == 0) ? "tsub" : "tadd";
      amount = -amount;
    }

  if ((amount % 16777216) != 0)
    return 0;

  scaled = amount / 16777216;
  if (scaled < 0 || scaled > 255)
    return 0;

  if (!tc32_same_reg_operand_p (dst, src))
    tmp = dst;
  else
    {
      if (amount <= 255)
        return 0;

      tmp = tc32_choose_low_scratch (dst_regno, src_regno);
    }

  imm_ops[0] = tmp;
  imm_ops[1] = GEN_INT (scaled);
  output_asm_insn ("tmov\t%0, %1", imm_ops);
  output_asm_insn ("tshftl\t%0, %0, #24", imm_ops);

  alu_ops[0] = dst;
  alu_ops[1] = src;
  alu_ops[2] = tmp;
  if (strcmp (op, "tadd") == 0)
    output_asm_insn ("tadd\t%0, %1, %2", alu_ops);
  else
    output_asm_insn ("tsub\t%0, %1, %2", alu_ops);

  return 1;
}

static int
tc32_emit_shiftable_const_addsub (const char *op, rtx *operands,
                                  HOST_WIDE_INT amount)
{
  unsigned HOST_WIDE_INT imm8;
  int shift;
  rtx tmp;
  rtx imm_ops[2];
  rtx alu_ops[3];
  rtx dst = tc32_strip_subreg (operands[0]);
  rtx src = tc32_strip_subreg (operands[1]);
  int dst_regno = REG_P (dst) ? REGNO (dst) : -1;
  int src_regno = REG_P (src) ? REGNO (src) : -1;

  if (amount == 0)
    return 0;

  if (amount < 0)
    {
      op = (strcmp (op, "tadd") == 0) ? "tsub" : "tadd";
      amount = -amount;
    }

  if (!tc32_split_shiftable_const (amount, &imm8, &shift))
    return 0;

  if (!tc32_same_reg_operand_p (dst, src))
    tmp = dst;
  else
    {
      if (amount <= 255)
        return 0;

      tmp = tc32_choose_low_scratch (dst_regno, src_regno);
    }

  imm_ops[0] = tmp;
  imm_ops[1] = GEN_INT (imm8);
  output_asm_insn ("tmov\t%0, %1", imm_ops);

  if (shift != 0)
    {
      char shift_asm[32];
      sprintf (shift_asm, "tshftl\t%%0, %%0, #%d", shift);
      output_asm_insn (shift_asm, imm_ops);
    }

  alu_ops[0] = dst;
  alu_ops[1] = src;
  alu_ops[2] = tmp;
  if (strcmp (op, "tadd") == 0)
    output_asm_insn ("tadd\t%0, %1, %2", alu_ops);
  else
    output_asm_insn ("tsub\t%0, %1, %2", alu_ops);

  return 1;
}

static int
tc32_emit_literal_const_addsub (const char *op, rtx *operands,
                                HOST_WIDE_INT amount)
{
  rtx tmp;
  rtx imm_ops[2];
  rtx alu_ops[3];
  rtx dst = tc32_strip_subreg (operands[0]);
  rtx src = tc32_strip_subreg (operands[1]);
  int dst_regno = REG_P (dst) ? REGNO (dst) : -1;
  int src_regno = REG_P (src) ? REGNO (src) : -1;

  if (amount == 0)
    return 0;

  if (!tc32_same_reg_operand_p (dst, src))
    tmp = dst;
  else
    tmp = tc32_choose_low_scratch (dst_regno, src_regno);

  imm_ops[0] = tmp;
  imm_ops[1] = GEN_INT (amount < 0 ? -amount : amount);
  tc32_emit_const_literal_load (tmp, imm_ops[1]);

  alu_ops[0] = dst;
  alu_ops[1] = src;
  alu_ops[2] = tmp;
  if ((strcmp (op, "tadd") == 0 && amount > 0)
      || (strcmp (op, "tsub") == 0 && amount < 0))
    output_asm_insn ("tadd\t%0, %1, %2", alu_ops);
  else
    output_asm_insn ("tsub\t%0, %1, %2", alu_ops);

  return 1;
}

HOST_WIDE_INT
tc32_simm32_intval (rtx x)
{
  return (HOST_WIDE_INT) (int) INTVAL (x);
}

const char *
tc32_output_movsi (rtx *operands)
{
  rtx src = operands[1];
  rtx dst = operands[0];

  if (GET_CODE (src) == SUBREG)
    src = SUBREG_REG (src);

  if (GET_CODE (dst) == SUBREG)
    dst = SUBREG_REG (dst);

  if (GET_CODE (src) == REG
      && (REGNO (src) == ARG_POINTER_REGNUM
          || REGNO (src) == FRAME_POINTER_REGNUM
          || REGNO (src) == STACK_POINTER_REGNUM
          || (REGNO (src) == HARD_FRAME_POINTER_REGNUM
              && !tc32_hard_frame_pointer_available_p ()))
      && GET_CODE (operands[0]) == REG)
    return tc32_output_movsi_core_from_special (operands);

  if (REG_P (dst) && REG_P (src))
    return "tmov\t%0, %1";

  if (GET_CODE (operands[0]) == REG
      && (GET_CODE (operands[1]) == SYMBOL_REF
          || GET_CODE (operands[1]) == LABEL_REF
          || GET_CODE (operands[1]) == CONST))
    {
      tc32_emit_const_literal_load (operands[0], operands[1]);
      return "";
    }

  if (GET_CODE (operands[0]) == REG
      && which_alternative == 2
      && GET_CODE (operands[1]) == MEM)
    {
      rtx addr = XEXP (operands[1], 0);

      if (GET_CODE (addr) == SYMBOL_REF
          || GET_CODE (addr) == LABEL_REF
          || GET_CODE (addr) == CONST)
        {
          if (tc32_hi_reg_p (operands[0]))
            {
              tc32_emit_const_literal_load (operands[0], addr);
              return "";
            }

          tc32_emit_const_literal_load (operands[0], addr);
          return "";
        }

      if (GET_CODE (addr) == REG
          || (GET_CODE (addr) == PLUS
              && REG_P (XEXP (addr, 0))
              && GET_CODE (XEXP (addr, 1)) == CONST_INT))
        {
          rtx base = (GET_CODE (addr) == REG) ? addr : XEXP (addr, 0);
          HOST_WIDE_INT addend = (GET_CODE (addr) == REG) ? 0
                                                          : INTVAL (XEXP (addr, 1));
          int regno = REGNO (base);
          HOST_WIDE_INT total = addend;
          rtx ops[3];

          {
            int resolved_regno
              = tc32_resolve_stack_base_offset (SImode, regno, addend, &total);

            if (resolved_regno != regno || total != addend)
              {
                regno = resolved_regno;
                base = gen_rtx_REG (SImode, regno);
                operands[1]
                  = gen_rtx_MEM (SImode,
                                 total ? plus_constant (base, total) : base);
              }
          }

          if (tc32_hi_reg_p (operands[0]) || tc32_hi_reg_p (base))
            {
              tc32_emit_lowreg_load ("tloadr\t%0, %1", operands[0], operands[1]);
              return "";
            }

          if ((regno == STACK_POINTER_REGNUM
               || regno == FRAME_POINTER_REGNUM
               || regno == HARD_FRAME_POINTER_REGNUM)
              && !tc32_legitimate_stack_mem_offset_p (SImode, regno, total))
            {
              ops[0] = operands[0];
              ops[1] = base;
              tc32_emit_chunked_addsub ("tadd", ops, total);
              output_asm_insn ("tloadr\t%0, [%0]", ops);
              return "";
            }
        }
    }

  if (GET_CODE (operands[0]) == MEM)
    {
      rtx addr = XEXP (operands[0], 0);
      rtx src = operands[1];
      rtx src_reg = src;
      int src_regno = REG_P (src) ? REGNO (src) : -1;

      if (GET_CODE (src_reg) == SUBREG)
        src_reg = SUBREG_REG (src_reg);

      if (REG_P (src_reg))
        src_regno = REGNO (src_reg);

      if (GET_CODE (addr) == SYMBOL_REF
          || GET_CODE (addr) == LABEL_REF
          || GET_CODE (addr) == CONST)
        {
          rtx addr_scratch = tc32_choose_low_scratch (src_regno, -1);
          rtx data = src;
          rtx ops[2];

          tc32_emit_const_literal_load (addr_scratch, addr);
          if (tc32_hi_reg_p (src))
            data = tc32_copy_to_low_reg (src, REGNO (addr_scratch), -1);
          ops[0] = addr_scratch;
          ops[1] = data;
          output_asm_insn ("tstorer\t%1, [%0]", ops);
          return "";
        }

      if (GET_CODE (addr) == REG
          || (GET_CODE (addr) == PLUS
              && REG_P (XEXP (addr, 0))
              && GET_CODE (XEXP (addr, 1)) == CONST_INT))
        {
          rtx base = (GET_CODE (addr) == REG) ? addr : XEXP (addr, 0);
          HOST_WIDE_INT addend = (GET_CODE (addr) == REG) ? 0
                                                          : INTVAL (XEXP (addr, 1));
          int regno = REGNO (base);
          HOST_WIDE_INT total = addend;

          {
            int resolved_regno
              = tc32_resolve_stack_base_offset (SImode, regno, addend, &total);

            if (resolved_regno != regno || total != addend)
              {
                regno = resolved_regno;
                base = gen_rtx_REG (SImode, regno);
                operands[0]
                  = gen_rtx_MEM (SImode,
                                 total ? plus_constant (base, total) : base);
              }
          }

          if (tc32_hi_reg_p (src) || tc32_hi_reg_p (base))
            {
              tc32_emit_lowreg_store ("tstorer\t%1, %0", operands[0], src);
              return "";
            }

          if ((regno == STACK_POINTER_REGNUM
               || regno == FRAME_POINTER_REGNUM
               || regno == HARD_FRAME_POINTER_REGNUM)
              && !tc32_legitimate_stack_mem_offset_p (SImode, regno, total))
            {
              rtx scratch = tc32_choose_low_scratch (src_regno, regno);
              rtx ops[2];

              ops[0] = scratch;
              ops[1] = base;
              tc32_emit_chunked_addsub ("tadd", ops, total);
              ops[1] = src;
              output_asm_insn ("tstorer\t%1, [%0]", ops);
              return "";
            }
        }
    }

  if (GET_CODE (operands[1]) == CONST_INT)
    {
      HOST_WIDE_INT value = INTVAL (operands[1]);
      unsigned HOST_WIDE_INT imm8;
      int shift;

      if (value < 0 && value >= -256)
        {
          fprintf (asm_out_file, "\ttmov\t%s, #%lu\n",
                   reg_names[REGNO (operands[0])],
                   (unsigned long) (-value - 1));
          fprintf (asm_out_file, "\ttmovn\t%s, %s\n",
                   reg_names[REGNO (operands[0])],
                   reg_names[REGNO (operands[0])]);
          return "";
        }

      if (value >= 0
          && value <= (HOST_WIDE_INT) 0xffffffffu
          && tc32_split_shiftable_const (value, &imm8, &shift))
        {
          fprintf (asm_out_file, "\ttmov\t%s, #%lu\n",
                   reg_names[REGNO (operands[0])], (unsigned long) imm8);
          if (shift != 0)
            fprintf (asm_out_file, "\ttshftl\t%s, %s, #%d\n",
                     reg_names[REGNO (operands[0])],
                     reg_names[REGNO (operands[0])],
                     shift);
          return "";
        }

      if (value < 0 || value > 255)
        {
          tc32_emit_const_literal_load (operands[0], operands[1]);
          return "";
        }
    }

  switch (which_alternative)
    {
    case 0:
    case 1:
    case 4:
      return "tmov\t%0, %1";

    case 2:
      return "tloadr\t%0, %1";

    default:
      return "tstorer\t%1, %0";
    }
}

const char *
tc32_output_movsi_core_from_special (rtx *operands)
{
  rtx dst = operands[0];
  rtx src = operands[1];

  if (GET_CODE (dst) == SUBREG)
    dst = SUBREG_REG (dst);

  if (GET_CODE (src) == SUBREG)
    src = SUBREG_REG (src);

  if (REGNO (dst) <= LAST_LO_REGNUM)
    return tc32_output_movsi_low_from_special (operands);

  if (REGNO (dst) >= 8 && REGNO (dst) <= 11)
    {
      rtx scratch = tc32_choose_low_scratch (REGNO (dst), -1);
      rtx ops[2];

      ops[0] = scratch;
      ops[1] = src;
      tc32_output_movsi_low_from_special (ops);
      ops[0] = dst;
      ops[1] = scratch;
      output_asm_insn ("tmov\t%0, %1", ops);
      return "";
    }

  return "tmov\t%0, %1";
}

const char *
tc32_output_movsi_low_from_special (rtx *operands)
{
  rtx src = operands[1];

  if (GET_CODE (src) == SUBREG)
    src = SUBREG_REG (src);

  if (GET_CODE (src) == REG
      && REGNO (src) == ARG_POINTER_REGNUM)
    {
      int base_regno = tc32_frame_access_base_regno ();
      HOST_WIDE_INT offset = tc32_initial_elimination_offset (ARG_POINTER_REGNUM,
                                                              base_regno);
      rtx ops[3];

      ops[0] = operands[0];
      ops[1] = gen_rtx_REG (SImode, base_regno);

      if (offset == 0)
        {
          operands[1] = ops[1];
          return "tmov\t%0, %1";
        }

      tc32_emit_chunked_addsub ("tadd", ops, offset);
      return "";
    }

  if (GET_CODE (src) == REG
      && REGNO (src) == FRAME_POINTER_REGNUM)
    {
      int base_regno = tc32_frame_access_base_regno ();
      HOST_WIDE_INT offset = tc32_initial_elimination_offset (FRAME_POINTER_REGNUM,
                                                              base_regno);
      rtx ops[3];

      ops[0] = operands[0];
      ops[1] = gen_rtx_REG (SImode, base_regno);

      if (offset == 0)
        {
          operands[1] = ops[1];
          return "tmov\t%0, %1";
        }

      tc32_emit_chunked_addsub ("tadd", ops, offset);
      return "";
    }

  if (GET_CODE (src) == REG
      && REGNO (src) == HARD_FRAME_POINTER_REGNUM
      && !tc32_hard_frame_pointer_available_p ())
    {
      operands[1] = stack_pointer_rtx;
      return "tmov\t%0, %1";
    }

  return "tmov\t%0, %1";
}

const char *
tc32_output_movhi (rtx *operands)
{
  if (GET_CODE (operands[0]) == REG
      && which_alternative == 2
      && GET_CODE (operands[1]) == MEM)
    {
      rtx addr = XEXP (operands[1], 0);

      if (GET_CODE (addr) == REG
          || (GET_CODE (addr) == PLUS
              && REG_P (XEXP (addr, 0))
              && GET_CODE (XEXP (addr, 1)) == CONST_INT))
        {
          rtx base = (GET_CODE (addr) == REG) ? addr : XEXP (addr, 0);
          HOST_WIDE_INT addend = (GET_CODE (addr) == REG) ? 0
                                                          : INTVAL (XEXP (addr, 1));
          int regno = REGNO (base);
          HOST_WIDE_INT total = addend;
          rtx ops[3];

          {
            int resolved_regno
              = tc32_resolve_stack_base_offset (HImode, regno, addend, &total);

            if (resolved_regno != regno || total != addend)
              {
                regno = resolved_regno;
                base = gen_rtx_REG (SImode, regno);
                operands[1]
                  = gen_rtx_MEM (HImode,
                                 total ? plus_constant (base, total) : base);
              }
          }

          if (tc32_hi_reg_p (operands[0]) || tc32_hi_reg_p (base))
            {
              tc32_emit_lowreg_load ("tloadrh\t%0, %1", operands[0], operands[1]);
              return "";
            }

          if ((regno == STACK_POINTER_REGNUM
               || regno == FRAME_POINTER_REGNUM
               || regno == HARD_FRAME_POINTER_REGNUM)
              && !tc32_legitimate_stack_mem_offset_p (HImode, regno, total))
            {
              ops[0] = operands[0];
              ops[1] = base;
              tc32_emit_chunked_addsub ("tadd", ops, total);
              output_asm_insn ("tloadrh\t%0, [%0]", ops);
              return "";
            }
        }
    }

  if (GET_CODE (operands[0]) == MEM)
    {
      rtx addr = XEXP (operands[0], 0);
      rtx src = operands[1];
      rtx src_reg = src;
      int src_regno = REG_P (src) ? REGNO (src) : -1;

      if (GET_CODE (src_reg) == SUBREG)
        src_reg = SUBREG_REG (src_reg);

      if (REG_P (src_reg))
        src_regno = REGNO (src_reg);

      if (GET_CODE (addr) == REG
          || (GET_CODE (addr) == PLUS
              && REG_P (XEXP (addr, 0))
              && GET_CODE (XEXP (addr, 1)) == CONST_INT))
        {
          rtx base = (GET_CODE (addr) == REG) ? addr : XEXP (addr, 0);
          HOST_WIDE_INT addend = (GET_CODE (addr) == REG) ? 0
                                                          : INTVAL (XEXP (addr, 1));
          int regno = REGNO (base);
          HOST_WIDE_INT total = addend;

          {
            int resolved_regno
              = tc32_resolve_stack_base_offset (HImode, regno, addend, &total);

            if (resolved_regno != regno || total != addend)
              {
                regno = resolved_regno;
                base = gen_rtx_REG (SImode, regno);
                operands[0]
                  = gen_rtx_MEM (HImode,
                                 total ? plus_constant (base, total) : base);
              }
          }

          if (tc32_hi_reg_p (src) || tc32_hi_reg_p (base))
            {
              tc32_emit_lowreg_store ("tstorerh\t%1, %0", operands[0], src);
              return "";
            }

          if ((regno == STACK_POINTER_REGNUM
               || regno == FRAME_POINTER_REGNUM
               || regno == HARD_FRAME_POINTER_REGNUM)
              && !tc32_legitimate_stack_mem_offset_p (HImode, regno, total))
            {
              rtx scratch = tc32_choose_low_scratch (src_regno, regno);
              rtx ops[2];

              ops[0] = scratch;
              ops[1] = base;
              tc32_emit_chunked_addsub ("tadd", ops, total);
              ops[1] = src;
              output_asm_insn ("tstorerh\t%1, [%0]", ops);
              return "";
            }
        }
    }

  if (GET_CODE (operands[1]) == CONST_INT)
    {
      HOST_WIDE_INT value = INTVAL (operands[1]);

      if (value < 0 || value > 255)
        {
          tc32_emit_const_literal_load (operands[0], operands[1]);
          return "";
        }
    }

  switch (which_alternative)
    {
    case 0:
    case 1:
    case 4:
      return "tmov\t%0, %1";

    case 2:
      return "tloadrh\t%0, %1";

    default:
      return "tstorerh\t%1, %0";
    }
}

const char *
tc32_output_movqi (rtx *operands)
{
  if (GET_CODE (operands[0]) == REG
      && which_alternative == 2
      && GET_CODE (operands[1]) == MEM)
    {
      rtx addr = XEXP (operands[1], 0);

      if (GET_CODE (addr) == REG
          || (GET_CODE (addr) == PLUS
              && REG_P (XEXP (addr, 0))
              && GET_CODE (XEXP (addr, 1)) == CONST_INT))
        {
          rtx base = (GET_CODE (addr) == REG) ? addr : XEXP (addr, 0);
          HOST_WIDE_INT addend = (GET_CODE (addr) == REG) ? 0
                                                          : INTVAL (XEXP (addr, 1));
          int regno = REGNO (base);
          HOST_WIDE_INT total = addend;
          rtx ops[3];

          {
            int resolved_regno
              = tc32_resolve_stack_base_offset (QImode, regno, addend, &total);

            if (resolved_regno != regno || total != addend)
              {
                regno = resolved_regno;
                base = gen_rtx_REG (SImode, regno);
                operands[1]
                  = gen_rtx_MEM (QImode,
                                 total ? plus_constant (base, total) : base);
              }
          }

          if (tc32_hi_reg_p (operands[0]) || tc32_hi_reg_p (base))
            {
              tc32_emit_lowreg_load ("tloadrb\t%0, %1", operands[0], operands[1]);
              return "";
            }

          if ((regno == STACK_POINTER_REGNUM
               || regno == FRAME_POINTER_REGNUM
               || regno == HARD_FRAME_POINTER_REGNUM)
              && !tc32_legitimate_stack_mem_offset_p (QImode, regno, total))
            {
              ops[0] = operands[0];
              ops[1] = base;
              tc32_emit_chunked_addsub ("tadd", ops, total);
              output_asm_insn ("tloadrb\t%0, [%0]", ops);
              return "";
            }
        }
    }

  if (GET_CODE (operands[0]) == MEM)
    {
      rtx addr = XEXP (operands[0], 0);
      rtx src = operands[1];
      rtx src_reg = src;
      int src_regno = REG_P (src) ? REGNO (src) : -1;

      if (GET_CODE (src_reg) == SUBREG)
        src_reg = SUBREG_REG (src_reg);

      if (REG_P (src_reg))
        src_regno = REGNO (src_reg);

      if (GET_CODE (addr) == REG
          || (GET_CODE (addr) == PLUS
              && REG_P (XEXP (addr, 0))
              && GET_CODE (XEXP (addr, 1)) == CONST_INT))
        {
          rtx base = (GET_CODE (addr) == REG) ? addr : XEXP (addr, 0);
          HOST_WIDE_INT addend = (GET_CODE (addr) == REG) ? 0
                                                          : INTVAL (XEXP (addr, 1));
          int regno = REGNO (base);
          HOST_WIDE_INT total = addend;

          {
            int resolved_regno
              = tc32_resolve_stack_base_offset (QImode, regno, addend, &total);

            if (resolved_regno != regno || total != addend)
              {
                regno = resolved_regno;
                base = gen_rtx_REG (SImode, regno);
                operands[0]
                  = gen_rtx_MEM (QImode,
                                 total ? plus_constant (base, total) : base);
              }
          }

          if (tc32_hi_reg_p (src) || tc32_hi_reg_p (base))
            {
              tc32_emit_lowreg_store ("tstorerb\t%1, %0", operands[0], src);
              return "";
            }

          if ((regno == STACK_POINTER_REGNUM
               || regno == FRAME_POINTER_REGNUM
               || regno == HARD_FRAME_POINTER_REGNUM)
              && !tc32_legitimate_stack_mem_offset_p (QImode, regno, total))
            {
              rtx scratch = tc32_choose_low_scratch (src_regno, regno);
              rtx ops[2];

              ops[0] = scratch;
              ops[1] = base;
              tc32_emit_chunked_addsub ("tadd", ops, total);
              ops[1] = src;
              output_asm_insn ("tstorerb\t%1, [%0]", ops);
              return "";
            }
        }
    }

  if (GET_CODE (operands[1]) == CONST_INT)
    {
      rtx ops[2];
      ops[0] = operands[0];
      ops[1] = GEN_INT (INTVAL (operands[1]) & 0xff);
      output_asm_insn ("tmov\t%0, %1", ops);
      return "";
    }

  switch (which_alternative)
    {
    case 0:
    case 4:
      return "tmov\t%0, %1";

    case 2:
      return "tloadrb\t%0, %1";

    default:
      return "tstorerb\t%1, %0";
    }
}

const char *
tc32_output_loadsi_special_neg_offset (rtx *operands)
{
  rtx addr = XEXP (operands[1], 0);
  rtx base = XEXP (addr, 0);
  rtx dst = operands[0];
  rtx low_dst = dst;
  rtx saved_dst = NULL_RTX;
  HOST_WIDE_INT delta = INTVAL (XEXP (addr, 1));
  rtx ops[3];

  if (tc32_hi_reg_p (dst))
    {
      low_dst = tc32_choose_low_scratch (REGNO (dst), REGNO (base));
      if (!tc32_reg_dead_here_p (REGNO (low_dst)))
        {
          saved_dst = low_dst;
          ops[0] = low_dst;
          output_asm_insn ("tpush\t{%0}", ops);
        }
    }

  ops[0] = low_dst;
  ops[1] = base;

  if (REG_P (base))
    {
      HOST_WIDE_INT total;
      int base_regno = tc32_resolve_stack_base_offset (SImode, REGNO (base),
                                                       delta, &total);

      if (base_regno != REGNO (base) || total != delta)
        {
          delta = total;
          ops[1] = gen_rtx_REG (SImode, base_regno);
        }
    }

  tc32_emit_chunked_addsub ("tadd", ops, delta);
  output_asm_insn ("tloadr\t%0, [%0]", ops);

  if (low_dst != dst)
    {
      ops[0] = dst;
      ops[1] = low_dst;
      output_asm_insn ("tmov\t%0, %1", ops);
    }

  if (saved_dst)
    {
      ops[0] = saved_dst;
      output_asm_insn ("tpop\t{%0}", ops);
    }

  return "";
}

const char *
tc32_output_addsi3 (rtx *operands)
{
  rtx dst = tc32_strip_subreg (operands[0]);
  rtx src1 = tc32_strip_subreg (operands[1]);
  rtx src2 = tc32_strip_subreg (operands[2]);
  rtx low_dst = dst;
  rtx low_src1 = src1;
  rtx low_src2 = src2;
  rtx saved_src1 = NULL_RTX;
  rtx saved_src2 = NULL_RTX;
  rtx ops[3];
  int dst_regno = (REG_P (dst) ? REGNO (dst) : -1);
  int src1_regno = (REG_P (src1) ? REGNO (src1) : -1);
  int src2_regno = (REG_P (src2) ? REGNO (src2) : -1);

  if (REG_P (src1)
      && (tc32_hi_reg_p (src1)
          || src1_regno == STACK_POINTER_REGNUM
          || src1_regno == FRAME_POINTER_REGNUM
          || src1_regno == ARG_POINTER_REGNUM
          || (src1_regno == HARD_FRAME_POINTER_REGNUM
              && !tc32_hard_frame_pointer_available_p ())))
    {
      if (src1_regno == STACK_POINTER_REGNUM
          || src1_regno == FRAME_POINTER_REGNUM
          || src1_regno == ARG_POINTER_REGNUM
          || (src1_regno == HARD_FRAME_POINTER_REGNUM
              && !tc32_hard_frame_pointer_available_p ()))
        {
          rtx tmp;
          rtx move_ops[2];

          if (REG_P (dst)
              && dst_regno >= 0
              && dst_regno <= LAST_LO_REGNUM)
            tmp = dst;
          else
            tmp = tc32_choose_low_scratch (dst_regno, src2_regno);

          move_ops[0] = tmp;
          move_ops[1] = src1;
          tc32_output_movsi_low_from_special (move_ops);
          low_src1 = tmp;
        }
      else
        {
          rtx move_ops[2];

          low_src1 = tc32_choose_low_scratch (dst_regno, src2_regno);
          saved_src1 = low_src1;
          move_ops[0] = low_src1;
          output_asm_insn ("tpush\t{%0}", move_ops);
          move_ops[1] = src1;
          output_asm_insn ("tmov\t%0, %1", move_ops);
        }
    }

  if (REG_P (src2)
      && (tc32_hi_reg_p (src2)
          || src2_regno == STACK_POINTER_REGNUM
          || src2_regno == FRAME_POINTER_REGNUM
          || src2_regno == ARG_POINTER_REGNUM
          || (src2_regno == HARD_FRAME_POINTER_REGNUM
              && !tc32_hard_frame_pointer_available_p ())))
    {
      if (src2_regno == STACK_POINTER_REGNUM
          || src2_regno == FRAME_POINTER_REGNUM
          || src2_regno == ARG_POINTER_REGNUM
          || (src2_regno == HARD_FRAME_POINTER_REGNUM
              && !tc32_hard_frame_pointer_available_p ()))
        {
          int avoid0 = (REG_P (low_src1) ? REGNO (low_src1) : src1_regno);
          rtx tmp;
          rtx move_ops[2];

          if (REG_P (dst)
              && dst_regno >= 0
              && dst_regno <= LAST_LO_REGNUM)
            tmp = dst;
          else
            tmp = tc32_choose_low_scratch (dst_regno, avoid0);

          move_ops[0] = tmp;
          move_ops[1] = src2;
          tc32_output_movsi_low_from_special (move_ops);
          low_src2 = tmp;
        }
      else
        {
          int avoid0 = (REG_P (low_src1) ? REGNO (low_src1) : src1_regno);
          rtx move_ops[2];

          low_src2 = tc32_choose_low_scratch (dst_regno, avoid0);
          saved_src2 = low_src2;
          move_ops[0] = low_src2;
          output_asm_insn ("tpush\t{%0}", move_ops);
          move_ops[1] = src2;
          output_asm_insn ("tmov\t%0, %1", move_ops);
        }
    }

  if (REG_P (dst)
      && tc32_hi_reg_p (dst))
    {
      int avoid0 = (REG_P (low_src1) ? REGNO (low_src1) : src1_regno);
      int avoid1 = (REG_P (low_src2) ? REGNO (low_src2) : src2_regno);
      if (avoid0 == dst_regno)
        avoid0 = -1;
      if (avoid1 == dst_regno)
        avoid1 = -1;
      low_dst = tc32_choose_low_scratch (avoid0, avoid1);
    }

  ops[0] = low_dst;
  ops[1] = low_src1;
  ops[2] = low_src2;

  if (GET_CODE (operands[2]) == CONST_INT)
    {
      HOST_WIDE_INT amount = tc32_simm32_intval (ops[2]);
      int same_regs = tc32_same_reg_operand_p (ops[0], ops[1]);

      if (amount < 0)
        {
          HOST_WIDE_INT abs_amount = -amount;

          if ((same_regs && abs_amount <= 255)
              || (!same_regs && abs_amount <= 7))
            {
              ops[2] = GEN_INT (abs_amount);
              output_asm_insn ("tsub\t%0, %1, %2", ops);
              goto finish;
            }
        }

      if (tc32_emit_shifted_byte_addsub ("tadd", ops, amount))
        goto finish;

      if (tc32_emit_shiftable_const_addsub ("tadd", ops, amount))
        goto finish;

      if (amount < 0
          || amount > 255
          || (!same_regs && amount > 7)
          || (amount >= 128 && !same_regs))
        {
          tc32_emit_literal_const_addsub ("tadd", ops, amount);
          goto finish;
        }
    }

  output_asm_insn ("tadd\t%0, %1, %2", ops);

finish:
  if (low_dst != dst)
    {
      rtx move_ops[2];
      move_ops[0] = dst;
      move_ops[1] = low_dst;
      output_asm_insn ("tmov\t%0, %1", move_ops);
    }

  if (saved_src2)
    {
      rtx move_ops[2];
      move_ops[0] = saved_src2;
      output_asm_insn ("tpop\t{%0}", move_ops);
    }

  if (saved_src1)
    {
      rtx move_ops[2];
      move_ops[0] = saved_src1;
      output_asm_insn ("tpop\t{%0}", move_ops);
    }
  return "";
}

const char *
tc32_output_addsi3_special_neg_mem (rtx *operands)
{
  static const int scratch_order[] = { 3, 2, 1, 0, 4, 5, 6 };
  rtx addr = XEXP (operands[2], 0);
  rtx base = XEXP (addr, 0);
  rtx tmp = operands[0];
  HOST_WIDE_INT delta = INTVAL (XEXP (addr, 1));
  int dst_regno = REGNO (operands[0]);
  int src_regno = REGNO (operands[1]);
  int base_regno = REGNO (base);
  int i;
  rtx ops[3];

  if (REG_P (base))
    {
      HOST_WIDE_INT total;
      int resolved_regno = tc32_resolve_stack_base_offset (SImode, base_regno,
                                                           delta, &total);

      if (resolved_regno != base_regno || total != delta)
        {
          base_regno = resolved_regno;
          delta = total;
          base = gen_rtx_REG (SImode, base_regno);
        }
    }

  if (dst_regno == src_regno)
    {
      tmp = NULL_RTX;

      for (i = 0; i < (int) ARRAY_SIZE (scratch_order); ++i)
        {
          int regno = scratch_order[i];

          if (regno == dst_regno || regno == base_regno)
            continue;

          if (find_regno_note (current_output_insn, REG_DEAD, regno))
            {
              tmp = gen_rtx_REG (SImode, regno);
              break;
            }
        }

      if (!tmp)
        for (i = 0; i < (int) ARRAY_SIZE (scratch_order); ++i)
          {
            int regno = scratch_order[i];

            if (regno == dst_regno || regno == base_regno)
              continue;

            tmp = gen_rtx_REG (SImode, regno);
            break;
          }
    }

  ops[0] = tmp;
  ops[1] = base;
  tc32_emit_chunked_addsub ("tadd", ops, delta);
  output_asm_insn ("tloadr\t%0, [%0]", ops);

  ops[0] = operands[0];
  ops[1] = operands[1];
  ops[2] = tmp;
  output_asm_insn ("tadd\t%0, %1, %2", ops);
  return "";
}

const char *
tc32_output_addsi3_symbol (rtx *operands)
{
  tc32_emit_const_literal_load (operands[0], operands[2]);
  output_asm_insn ("tadd\t%0, %0, %1", operands);
  return "";
}

const char *
tc32_output_subsi3_symbol (rtx *operands)
{
  tc32_emit_const_literal_load (operands[0], operands[2]);
  output_asm_insn ("tsub\t%0, %1, %0", operands);
  return "";
}

const char *
tc32_output_jump (rtx insn, int labelno)
{
  static char buffer[160];
  unsigned long litno;

  if (get_attr_length (insn) <= 2)
    sprintf (buffer, "tj\t%%l%d", labelno);
  else if (get_attr_length (insn) <= 4)
    sprintf (buffer, "tjl\t%%l%d", labelno);
  else
    {
      litno = tc32_literal_label_num++;
      sprintf (buffer,
               "tloadr\tr3, .LTC32J%lu\n\ttjex\tr3\n\t.align\t2\n.LTC32J%lu:\n\t.word\t%%l%d",
               litno, litno, labelno);
    }
  return buffer;
}

const char *
tc32_output_cond_jump_tail (rtx insn, int labelno)
{
  static char buffer[160];
  unsigned long litno;

  if (get_attr_length (insn) <= 4)
    sprintf (buffer, "tj\t%%l%d", labelno);
  else
    {
      litno = tc32_literal_label_num++;
      sprintf (buffer,
               "tloadr\tr3, .LTC32J%lu\n\ttjex\tr3\n\t.align\t2\n.LTC32J%lu:\n\t.word\t%%l%d",
               litno, litno, labelno);
    }
  return buffer;
}

const char *
tc32_output_cbranch (rtx insn, int code, int labelno)
{
  static char buffer[224];
  const char *mnemonic;
  const char *jump_mnemonic;
  unsigned long litno;

  switch (code)
    {
    case EQ:
      mnemonic = "tjne";
      break;
    case NE:
      mnemonic = "tjeq";
      break;
    case LT:
      mnemonic = "tjge";
      break;
    case LE:
      mnemonic = "tjgt";
      break;
    case GT:
      mnemonic = "tjle";
      break;
    case GE:
      mnemonic = "tjlt";
      break;
    case LTU:
      mnemonic = "tjhs";
      break;
    case LEU:
      mnemonic = "tjhi";
      break;
    case GTU:
      mnemonic = "tjls";
      break;
    case GEU:
      mnemonic = "tjlo";
      break;
    default:
      gcc_unreachable ();
    }

  if (get_attr_length (insn) <= 4)
    jump_mnemonic = "tj";
  else
    jump_mnemonic = 0;

  if (jump_mnemonic)
    sprintf (buffer,
             "%s\t.LTC32CB%lu\n\t%s\t%%l%d\n.LTC32CB%lu:",
             mnemonic, tc32_cbranch_label_num,
             jump_mnemonic, labelno, tc32_cbranch_label_num);
  else
    {
      litno = tc32_literal_label_num++;
      sprintf (buffer,
               "%s\t.LTC32CB%lu\n\ttloadr\tr3, .LTC32J%lu\n\ttjex\tr3\n\t.align\t2\n.LTC32J%lu:\n\t.word\t%%l%d\n.LTC32CB%lu:",
               mnemonic, tc32_cbranch_label_num,
               litno, litno, labelno, tc32_cbranch_label_num);
    }
  tc32_cbranch_label_num++;
  return buffer;
}

const char *
tc32_output_tbit_cbranch (rtx insn, int code, int labelno)
{
  static char buffer[224];
  const char *skip_mnemonic;
  const char *jump_mnemonic;
  unsigned long litno;

  switch (code)
    {
    case EQ:
      skip_mnemonic = "tjmi";
      break;
    case NE:
      skip_mnemonic = "tjpl";
      break;
    default:
      gcc_unreachable ();
    }

  if (get_attr_length (insn) <= 4)
    jump_mnemonic = "tj";
  else
    jump_mnemonic = 0;

  if (jump_mnemonic)
    sprintf (buffer,
             "%s\t.LTC32CB%lu\n\t%s\t%%l%d\n.LTC32CB%lu:",
             skip_mnemonic, tc32_cbranch_label_num,
             jump_mnemonic, labelno, tc32_cbranch_label_num);
  else
    {
      litno = tc32_literal_label_num++;
      sprintf (buffer,
               "%s\t.LTC32CB%lu\n\ttloadr\tr3, .LTC32J%lu\n\ttjex\tr3\n\t.align\t2\n.LTC32J%lu:\n\t.word\t%%l%d\n.LTC32CB%lu:",
               skip_mnemonic, tc32_cbranch_label_num,
               litno, litno, labelno, tc32_cbranch_label_num);
    }
  tc32_cbranch_label_num++;
  return buffer;
}

const char *
tc32_output_subsi3 (rtx *operands)
{
  rtx canon_ops[3];

  canon_ops[0] = tc32_strip_subreg (operands[0]);
  canon_ops[1] = tc32_strip_subreg (operands[1]);
  canon_ops[2] = tc32_strip_subreg (operands[2]);

  if (GET_CODE (canon_ops[2]) == CONST_INT)
    {
      HOST_WIDE_INT amount = tc32_simm32_intval (canon_ops[2]);
      int same_regs = tc32_same_reg_operand_p (canon_ops[0], canon_ops[1]);

      if (amount < 0)
        {
          HOST_WIDE_INT abs_amount = -amount;

          if ((same_regs && abs_amount <= 255)
              || (!same_regs && abs_amount <= 7))
            {
              canon_ops[2] = GEN_INT (abs_amount);
              output_asm_insn ("tadd\t%0, %1, %2", canon_ops);
              return "";
            }
        }

      if (tc32_emit_shifted_byte_addsub ("tsub", canon_ops, amount))
        return "";

      if (tc32_emit_shiftable_const_addsub ("tsub", canon_ops, amount))
        return "";

      if (amount < 0
          || amount > 255
          || (!same_regs && amount > 7)
          || (amount >= 128 && !same_regs))
        {
          tc32_emit_literal_const_addsub ("tsub", canon_ops, amount);
          return "";
        }
    }

  return "tsub\t%0, %1, %2";
}

void
thumb_set_frame_pointer (void)
{
  struct tc32_stack_offsets *offsets;
  HOST_WIDE_INT amount;
  rtx insn;

  offsets = thumb_get_frame_offsets ();
  amount = offsets->outgoing_args - offsets->locals_base;

  if (amount <= 255)
    insn = emit_insn (gen_addsi3 (hard_frame_pointer_rtx,
                                  stack_pointer_rtx, GEN_INT (amount)));
  else
    {
      emit_insn (gen_movsi (hard_frame_pointer_rtx, GEN_INT (amount)));
      insn = emit_insn (gen_addsi3 (hard_frame_pointer_rtx,
                                    hard_frame_pointer_rtx,
                                    stack_pointer_rtx));
    }

  RTX_FRAME_RELATED_P (insn) = 1;
}

void
thumb1_output_function_prologue (FILE *f, HOST_WIDE_INT size ATTRIBUTE_UNUSED)
{
  struct tc32_stack_offsets *offsets;
  unsigned long live_regs_mask;
  unsigned long l_mask;
  unsigned high_regs_pushed;
  HOST_WIDE_INT amount;
  int regno;

  offsets = thumb_get_frame_offsets ();
  live_regs_mask = offsets->saved_regs_mask;
  l_mask = live_regs_mask & 0x40ff;
  high_regs_pushed = tc32_bit_count (live_regs_mask & 0x0f00);

  if ((l_mask & 0xff) != 0
      || (high_regs_pushed == 0 && l_mask))
    thumb_pushpop (f, l_mask, 1, NULL, l_mask);

  if (high_regs_pushed)
    {
      unsigned pushable_regs;
      unsigned next_hi_reg;

      for (next_hi_reg = 11; next_hi_reg > LAST_LO_REGNUM; next_hi_reg--)
        if (live_regs_mask & (1U << next_hi_reg))
          break;

      pushable_regs = l_mask & 0xff;

      if (pushable_regs == 0)
        pushable_regs = 1U << thumb_find_work_register (live_regs_mask);

      while (high_regs_pushed > 0)
        {
          unsigned long real_regs_mask = 0;

          for (regno = LAST_LO_REGNUM; regno >= 0; regno--)
            {
              if (pushable_regs & (1U << regno))
                {
                  fprintf (f, "\ttmov\t%s, %s\n",
                           reg_names[regno], reg_names[next_hi_reg]);

                  high_regs_pushed--;
                  real_regs_mask |= 1U << next_hi_reg;

                  if (high_regs_pushed)
                    {
                      for (next_hi_reg--; next_hi_reg > LAST_LO_REGNUM;
                           next_hi_reg--)
                        if (live_regs_mask & (1U << next_hi_reg))
                          break;
                    }
                  else
                    {
                      pushable_regs &= ~((1U << regno) - 1U);
                      break;
                    }
                }
            }

          if (l_mask == (1U << RETURN_ADDRESS_POINTER_REGNUM))
            {
              thumb_pushpop (f, pushable_regs | (1U << RETURN_ADDRESS_POINTER_REGNUM),
                             1, NULL,
                             real_regs_mask | (1U << RETURN_ADDRESS_POINTER_REGNUM));
              l_mask = 0;
            }
          else
            thumb_pushpop (f, pushable_regs, 1, NULL, real_regs_mask);
        }
    }

      amount = offsets->outgoing_args - offsets->saved_regs;
  if (amount > 0)
    tc32_output_sp_adjust (f, -amount, live_regs_mask);

  if (tc32_real_frame_pointer_needed ())
    {
      amount = offsets->outgoing_args - offsets->locals_base;
      if (amount <= 255)
        fprintf (f, "\ttadd\t%s, %s, #%ld\n",
                 reg_names[HARD_FRAME_POINTER_REGNUM],
                 reg_names[STACK_POINTER_REGNUM],
                 (long) amount);
      else
        {
          fprintf (f, "\ttmov\t%s, #255\n", reg_names[HARD_FRAME_POINTER_REGNUM]);
          amount -= 255;
          while (amount > 0)
            {
              HOST_WIDE_INT step = amount > 255 ? 255 : amount;
              fprintf (f, "\ttadd\t%s, %s, #%ld\n",
                       reg_names[HARD_FRAME_POINTER_REGNUM],
                       reg_names[HARD_FRAME_POINTER_REGNUM],
                       (long) step);
              amount -= step;
            }
          fprintf (f, "\ttadd\t%s, %s, %s\n",
                   reg_names[HARD_FRAME_POINTER_REGNUM],
                   reg_names[HARD_FRAME_POINTER_REGNUM],
                   reg_names[STACK_POINTER_REGNUM]);
        }
    }
}

const char *
thumb_unexpanded_epilogue (void)
{
  struct tc32_stack_offsets *offsets;
  unsigned long live_regs_mask;
  int high_regs_pushed;
  int had_to_push_lr;
  unsigned size;
  HOST_WIDE_INT amount;
  int regno;

  offsets = thumb_get_frame_offsets ();
  live_regs_mask = offsets->saved_regs_mask;
  high_regs_pushed = tc32_bit_count (live_regs_mask & 0x0f00);
  had_to_push_lr = (live_regs_mask & (1U << RETURN_ADDRESS_POINTER_REGNUM)) != 0;
  size = tc32_size_return_regs ();

  if (tc32_real_frame_pointer_needed ())
    {
      fprintf (asm_out_file, "\ttmov\t%s, %s\n",
               reg_names[STACK_POINTER_REGNUM],
               reg_names[HARD_FRAME_POINTER_REGNUM]);
      amount = offsets->locals_base - offsets->saved_regs;
    }
  else
    amount = offsets->outgoing_args - offsets->saved_regs;

  if (amount > 0)
    tc32_output_sp_adjust (asm_out_file, amount, offsets->saved_regs_mask);

  if (high_regs_pushed)
    {
      unsigned long mask = live_regs_mask & 0xff;
      int next_hi_reg;

      if (size <= 12)
        mask |= 1U << 3;
      if (size <= 8)
        mask |= 1U << 2;

      if (mask == 0)
        internal_error ("no low registers available for popping high registers");

      for (next_hi_reg = 8; next_hi_reg < 12; next_hi_reg++)
        if (live_regs_mask & (1U << next_hi_reg))
          break;

      while (high_regs_pushed)
        {
          for (regno = 0; regno <= LAST_LO_REGNUM; regno++)
            {
              if (mask & (1U << regno))
                high_regs_pushed--;
              if (high_regs_pushed == 0)
                break;
            }

          mask &= (2U << regno) - 1U;

          thumb_pushpop (asm_out_file, mask, 0, NULL, mask);

          for (regno = 0; regno <= LAST_LO_REGNUM; regno++)
            if (mask & (1U << regno))
              {
                fprintf (asm_out_file, "\ttmov\t%s, %s\n",
                         reg_names[next_hi_reg], reg_names[regno]);

                for (next_hi_reg++; next_hi_reg < 12; next_hi_reg++)
                  if (live_regs_mask & (1U << next_hi_reg))
                    break;
              }
        }

      live_regs_mask &= ~0x0f00;
    }

  live_regs_mask &= 0xff;

  if (crtl->args.pretend_args_size == 0)
    {
      if (had_to_push_lr)
        live_regs_mask |= 1U << TC32_PC_REGNUM;

      if (live_regs_mask)
        thumb_pushpop (asm_out_file, live_regs_mask, 0, NULL, live_regs_mask);

      if (!had_to_push_lr)
        thumb_exit (asm_out_file, RETURN_ADDRESS_POINTER_REGNUM);
    }
  else
    {
      int regno;

      if (live_regs_mask)
        thumb_pushpop (asm_out_file, live_regs_mask, 0, NULL, live_regs_mask);

      if (had_to_push_lr)
        {
          if (size > 12)
            fprintf (asm_out_file, "\ttmov\t%s, %s\n",
                     reg_names[TC32_IP_REGNUM], reg_names[LAST_ARG_REGNUM]);

          thumb_pushpop (asm_out_file, 1U << LAST_ARG_REGNUM, 0, NULL,
                         1U << LAST_ARG_REGNUM);

          if (size > 12)
            {
              fprintf (asm_out_file, "\ttmov\t%s, %s\n",
                       reg_names[RETURN_ADDRESS_POINTER_REGNUM],
                       reg_names[LAST_ARG_REGNUM]);
              fprintf (asm_out_file, "\ttmov\t%s, %s\n",
                       reg_names[LAST_ARG_REGNUM], reg_names[TC32_IP_REGNUM]);
              regno = RETURN_ADDRESS_POINTER_REGNUM;
            }
          else
            regno = LAST_ARG_REGNUM;
        }
      else
        regno = RETURN_ADDRESS_POINTER_REGNUM;

      fprintf (asm_out_file, "\ttadd\t%s, %s, #%d\n",
               reg_names[TC32_SP_REGNUM], reg_names[TC32_SP_REGNUM],
               crtl->args.pretend_args_size);

      thumb_exit (asm_out_file, regno);
    }

  return "";
}

void
thumb_set_return_address (rtx source, rtx scratch)
{
  struct tc32_stack_offsets *offsets;
  unsigned long mask;
  HOST_WIDE_INT delta;
  rtx addr;
  rtx mem;
  int reg;
  int regno;

  emit_use (source);

  offsets = thumb_get_frame_offsets ();
  mask = offsets->saved_regs_mask;

  if (!(mask & (1U << TC32_LR_REGNUM)))
    {
      emit_move_insn (gen_rtx_REG (Pmode, TC32_LR_REGNUM), source);
      return;
    }

  if (tc32_real_frame_pointer_needed ())
    {
      delta = offsets->locals_base - offsets->saved_args;
      reg = HARD_FRAME_POINTER_REGNUM;
    }
  else
    {
      delta = offsets->outgoing_args - offsets->saved_args;
      reg = STACK_POINTER_REGNUM;
    }

  delta -= offsets->outgoing_args - offsets->saved_regs;

  for (regno = 0; regno < TC32_LR_REGNUM; regno++)
    if (mask & (1U << regno))
      delta -= UNITS_PER_WORD;

  delta -= UNITS_PER_WORD;

  addr = gen_rtx_REG (SImode, reg);
  if (delta != 0)
    {
      emit_insn (gen_movsi (scratch, GEN_INT (delta)));
      emit_insn (gen_addsi3 (scratch, scratch, addr));
      addr = scratch;
    }

  mem = gen_rtx_MEM (Pmode, addr);
  emit_move_insn (mem, source);
}

const char *
thumb_call_via_reg (rtx reg)
{
  static char buffer[128];
  unsigned long labelno = tc32_indirect_call_label_num++;
  int regno;

  gcc_assert (REGNO (reg) < RETURN_ADDRESS_POINTER_REGNUM);
  regno = REGNO (reg);

  if (regno == TC32_IP_REGNUM)
    sprintf (buffer,
             "tjl\t.LTC32CALL%lu\n\ttj\t.LTC32RET%lu\n.LTC32CALL%lu:\n\ttjex\t%s\n.LTC32RET%lu:",
             labelno, labelno, labelno, reg_names[regno], labelno);
  else
    sprintf (buffer,
             "tmov\t%s, %s\n\ttjl\t.LTC32CALL%lu\n\ttj\t.LTC32RET%lu\n.LTC32CALL%lu:\n\ttjex\t%s\n.LTC32RET%lu:",
             reg_names[TC32_IP_REGNUM], reg_names[regno],
             labelno, labelno, labelno, reg_names[TC32_IP_REGNUM], labelno);
  return buffer;
}

const char *
thumb_load_double_from_address (rtx *operands)
{
  rtx addr;

  gcc_assert (GET_CODE (operands[0]) == REG);
  gcc_assert (GET_CODE (operands[1]) == MEM);

  addr = XEXP (operands[1], 0);

  switch (GET_CODE (addr))
    {
    case REG:
    case PLUS:
    case CONST:
    case LABEL_REF:
    case SYMBOL_REF:
      if (GET_CODE (addr) == REG || GET_CODE (addr) == PLUS)
        operands[2] = adjust_address (operands[1], SImode, 4);
      else
        operands[2] = gen_rtx_MEM (SImode, plus_constant (copy_rtx (addr), 4));

      if (GET_CODE (addr) == REG && REGNO (operands[0]) == REGNO (addr))
        {
          output_asm_insn ("tloadr\t%H0, %2", operands);
          output_asm_insn ("tloadr\t%0, %1", operands);
        }
      else
        {
          output_asm_insn ("tloadr\t%0, %1", operands);
          output_asm_insn ("tloadr\t%H0, %2", operands);
        }
      break;

    default:
      gcc_unreachable ();
    }

  return "";
}

const char *
tc32_output_loadsi_stack (rtx *operands)
{
  rtx base = operands[1];

  if (REG_P (base))
    {
      HOST_WIDE_INT offset;
      int base_regno = tc32_resolve_stack_base_offset (SImode, REGNO (base), 0,
                                                       &offset);
      rtx addr = gen_rtx_REG (SImode, base_regno);

      if (offset
          && !tc32_legitimate_stack_mem_offset_p (SImode, base_regno, offset))
        {
          rtx ops[3];

          ops[0] = operands[0];
          ops[1] = gen_rtx_REG (SImode, base_regno);
          tc32_emit_chunked_addsub ("tadd", ops, offset);
          output_asm_insn ("tloadr\t%0, [%0]", ops);
          return "";
        }

      operands[2] = gen_rtx_MEM (SImode, offset ? plus_constant (addr, offset)
                                                : addr);
      output_asm_insn ("tloadr\t%0, %2", operands);
      return "";
    }

  return "tloadr\t%0, [%1]";
}

const char *
tc32_output_storesi_stack (rtx *operands)
{
  rtx base = operands[0];

  if (REG_P (base))
    {
      HOST_WIDE_INT offset;
      int base_regno = tc32_resolve_stack_base_offset (SImode, REGNO (base), 0,
                                                       &offset);
      rtx addr = gen_rtx_REG (SImode, base_regno);

      if (offset)
        addr = plus_constant (addr, offset);

      operands[2] = gen_rtx_MEM (SImode, addr);
      output_asm_insn ("tstorer\t%1, %2", operands);
      return "";
    }

  return "tstorer\t%1, [%0]";
}

const char *
tc32_output_movsi_memmem (rtx *operands)
{
  rtx scratch = tc32_choose_low_scratch (-1, -1);
  rtx ops[2];
  int scratch_regno = REGNO (scratch);

  if (!tc32_reg_dead_here_p (scratch_regno))
    {
      ops[0] = scratch;
      output_asm_insn ("tpush\t{%0}", ops);
    }

  tc32_emit_lowreg_load ("tloadr\t%0, %1", scratch, operands[1]);
  tc32_emit_lowreg_store ("tstorer\t%1, %0", operands[0], scratch);

  if (!tc32_reg_dead_here_p (scratch_regno))
    {
      ops[0] = scratch;
      output_asm_insn ("tpop\t{%0}", ops);
    }

  return "";
}

const char *
thumb_output_move_mem_multiple (int n, rtx *operands)
{
  rtx tmp;

  switch (n)
    {
    case 2:
      if (REGNO (operands[4]) > REGNO (operands[5]))
        {
          tmp = operands[4];
          operands[4] = operands[5];
          operands[5] = tmp;
        }
      output_asm_insn ("tloadm\t%1!, {%4, %5}", operands);
      output_asm_insn ("tstorem\t%0!, {%4, %5}", operands);
      break;

    case 3:
      if (REGNO (operands[4]) > REGNO (operands[5]))
        {
          tmp = operands[4];
          operands[4] = operands[5];
          operands[5] = tmp;
        }
      if (REGNO (operands[5]) > REGNO (operands[6]))
        {
          tmp = operands[5];
          operands[5] = operands[6];
          operands[6] = tmp;
        }
      if (REGNO (operands[4]) > REGNO (operands[5]))
        {
          tmp = operands[4];
          operands[4] = operands[5];
          operands[5] = tmp;
        }
      output_asm_insn ("tloadm\t%1!, {%4, %5, %6}", operands);
      output_asm_insn ("tstorem\t%0!, {%4, %5, %6}", operands);
      break;

    default:
      gcc_unreachable ();
    }

  return "";
}

void
thumb_expand_movmemqi (rtx *operands)
{
  rtx out = copy_to_mode_reg (SImode, XEXP (operands[0], 0));
  rtx in = copy_to_mode_reg (SImode, XEXP (operands[1], 0));
  HOST_WIDE_INT len = INTVAL (operands[2]);
  HOST_WIDE_INT offset = 0;

  while (len >= 12)
    {
      emit_insn (gen_movmem12b (out, in, out, in));
      len -= 12;
    }

  if (len >= 8)
    {
      emit_insn (gen_movmem8b (out, in, out, in));
      len -= 8;
    }

  if (len >= 4)
    {
      rtx reg = gen_reg_rtx (SImode);
      emit_insn (gen_movsi (reg, gen_rtx_MEM (SImode, in)));
      emit_insn (gen_movsi (gen_rtx_MEM (SImode, out), reg));
      len -= 4;
      offset += 4;
    }

  if (len >= 2)
    {
      rtx reg = gen_reg_rtx (HImode);
      emit_insn (gen_movhi (reg, gen_rtx_MEM (HImode,
                                              plus_constant (in, offset))));
      emit_insn (gen_movhi (gen_rtx_MEM (HImode,
                                         plus_constant (out, offset)),
                            reg));
      len -= 2;
      offset += 2;
    }

  if (len)
    {
      rtx reg = gen_reg_rtx (QImode);
      emit_insn (gen_movqi (reg, gen_rtx_MEM (QImode,
                                              plus_constant (in, offset))));
      emit_insn (gen_movqi (gen_rtx_MEM (QImode,
                                         plus_constant (out, offset)),
                            reg));
    }
}

void
thumb_reload_out_hi (rtx *operands)
{
  emit_insn (gen_thumb_movhi_clobber (operands[0], operands[1], operands[2]));
}

void
thumb_reload_in_hi (rtx *operands)
{
  emit_insn (gen_movhi (operands[0], operands[1]));
}

const char *
thumb1_output_casesi (rtx *operands)
{
  rtx diff_vec = PATTERN (next_real_insn (operands[0]));
  enum rtx_code code = GET_CODE (diff_vec);

  gcc_assert (code == ADDR_DIFF_VEC || code == ADDR_VEC);

  switch (GET_MODE (diff_vec))
    {
    case QImode:
      return (code == ADDR_DIFF_VEC
              && ADDR_DIFF_VEC_FLAGS (diff_vec).offset_unsigned
              ? "tjl\t___gnu_thumb1_case_uqi"
              : "tjl\t___gnu_thumb1_case_sqi");

    case HImode:
      return (code == ADDR_DIFF_VEC
              && ADDR_DIFF_VEC_FLAGS (diff_vec).offset_unsigned
              ? "tjl\t___gnu_thumb1_case_uhi"
              : "tjl\t___gnu_thumb1_case_shi");

    case SImode:
      if (code == ADDR_VEC)
        {
          /* The vendor tc32 backend dispatches 32-bit jump tables directly
             through an absolute-address table instead of calling the generic
             Thumb helper.  Matching that shape avoids LR-dependent helper
             flow and makes tc32 dense switches closer to vendor codegen.  */
          output_asm_insn ("tshftl\tr0, r0, #2", operands);
          output_asm_insn ("tloadr\tr1, %0", operands);
          output_asm_insn ("tloadr\tr0, [r1, r0]", operands);
          output_asm_insn ("tmov\tpc, r0", operands);
          return "";
        }

      return "tjl\t___gnu_thumb1_case_si";

    default:
      gcc_unreachable ();
    }
}
