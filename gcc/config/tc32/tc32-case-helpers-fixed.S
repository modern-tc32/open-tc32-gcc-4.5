.text
.align 2

.global ____gnu_thumb1_case_sqi
____gnu_thumb1_case_sqi:
	tpush	{r1}
	tmov	r1, lr
	tshftr	r1, r1, #1
	tshftl	r1, r1, #1
	tloadrsb	r1, [r1, r0]
	tshftl	r1, r1, #1
	tadd	lr, r1
	tpop	{r1}
	tjex	lr

.global ____gnu_thumb1_case_uqi
____gnu_thumb1_case_uqi:
	tpush	{r1}
	tmov	r1, lr
	tshftr	r1, r1, #1
	tshftl	r1, r1, #1
	tloadrb	r1, [r1, r0]
	tshftl	r1, r1, #1
	tadd	lr, r1
	tpop	{r1}
	tjex	lr

.global ____gnu_thumb1_case_shi
____gnu_thumb1_case_shi:
	tpush	{r0, r1}
	tmov	r1, lr
	tshftr	r1, r1, #1
	tshftl	r0, r0, #1
	tshftl	r1, r1, #1
	tloadrsh	r1, [r1, r0]
	tshftl	r1, r1, #1
	tadd	lr, r1
	tpop	{r0, r1}
	tjex	lr

.global ____gnu_thumb1_case_uhi
____gnu_thumb1_case_uhi:
	tpush	{r0, r1}
	tmov	r1, lr
	tshftr	r1, r1, #1
	tshftl	r0, r0, #1
	tshftl	r1, r1, #1
	tloadrh	r1, [r1, r0]
	tshftl	r1, r1, #1
	tadd	lr, r1
	tpop	{r0, r1}
	tjex	lr

.global ____gnu_thumb1_case_si
____gnu_thumb1_case_si:
	tpush	{r0, r1}
	tmov	r1, lr
	tadd	r1, #2
	tshftr	r1, r1, #2
	tshftl	r0, r0, #2
	tshftl	r1, r1, #2
	tloadr	r0, [r1, r0]
	tmov	lr, r0
	tpop	{r0, r1}
	tmov	pc, lr
