;; Predicate definitions for TC32, derived from ARM/Thumb1.

(define_predicate "s_register_operand"
  (match_code "reg,subreg")
{
  if (GET_CODE (op) == SUBREG)
    op = SUBREG_REG (op);

  return (GET_CODE (op) == REG
          && (REGNO (op) >= FIRST_PSEUDO_REGISTER
              || REGNO_REG_CLASS (REGNO (op)) != NO_REGS));
})

(define_predicate "low_register_operand"
  (and (match_code "reg")
       (match_test "REGNO (op) <= LAST_LO_REGNUM")))

(define_predicate "reg_or_int_operand"
  (ior (match_code "const_int")
       (match_operand 0 "s_register_operand")))

(define_special_predicate "multi_register_push"
  (match_code "parallel")
{
  if ((GET_CODE (XVECEXP (op, 0, 0)) != SET)
      || (GET_CODE (SET_SRC (XVECEXP (op, 0, 0))) != UNSPEC)
      || (XINT (SET_SRC (XVECEXP (op, 0, 0)), 1) != UNSPEC_PUSH_MULT))
    return false;

  return true;
})

(define_predicate "thumb1_cmp_operand"
  (ior (and (match_code "reg,subreg")
            (match_operand 0 "s_register_operand"))
       (and (match_code "const_int")
            (match_test "((unsigned HOST_WIDE_INT) INTVAL (op)) < 256"))))

(define_predicate "thumb1_cmpneg_operand"
  (and (match_code "const_int")
       (match_test "INTVAL (op) < 0 && INTVAL (op) > -256")))

(define_predicate "thumb_cbrch_target_operand"
  (and (match_code "reg,subreg,mem")
       (ior (match_operand 0 "s_register_operand")
            (and (match_test "reload_in_progress || reload_completed")
                 (match_operand 0 "memory_operand")))))
