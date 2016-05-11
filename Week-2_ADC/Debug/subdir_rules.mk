################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
MS2ID.obj: ../MS2ID.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"c:/ti/ccsv6/tools/compiler/arm_5.1.6/bin/armcl" -mv7M3 --code_state=16 --abi=eabi -me -g --include_path="c:/ti/ccsv6/tools/compiler/arm_5.1.6/include" --include_path="P:/courses/ENCE361/StellarisWare/utils" --include_path="P:/courses/ENCE361/StellarisWare/boards/ek-lm3s1968" --include_path="P:/courses/ENCE361/StellarisWare" --diag_warning=225 --display_error_number --diag_wrap=off --preproc_with_compile --preproc_dependency="MS2ID.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

button.obj: ../button.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"c:/ti/ccsv6/tools/compiler/arm_5.1.6/bin/armcl" -mv7M3 --code_state=16 --abi=eabi -me -g --include_path="c:/ti/ccsv6/tools/compiler/arm_5.1.6/include" --include_path="P:/courses/ENCE361/StellarisWare/utils" --include_path="P:/courses/ENCE361/StellarisWare/boards/ek-lm3s1968" --include_path="P:/courses/ENCE361/StellarisWare" --diag_warning=225 --display_error_number --diag_wrap=off --preproc_with_compile --preproc_dependency="button.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

buttonSet.obj: ../buttonSet.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"c:/ti/ccsv6/tools/compiler/arm_5.1.6/bin/armcl" -mv7M3 --code_state=16 --abi=eabi -me -g --include_path="c:/ti/ccsv6/tools/compiler/arm_5.1.6/include" --include_path="P:/courses/ENCE361/StellarisWare/utils" --include_path="P:/courses/ENCE361/StellarisWare/boards/ek-lm3s1968" --include_path="P:/courses/ENCE361/StellarisWare" --diag_warning=225 --display_error_number --diag_wrap=off --preproc_with_compile --preproc_dependency="buttonSet.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

circBuf.obj: ../circBuf.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"c:/ti/ccsv6/tools/compiler/arm_5.1.6/bin/armcl" -mv7M3 --code_state=16 --abi=eabi -me -g --include_path="c:/ti/ccsv6/tools/compiler/arm_5.1.6/include" --include_path="P:/courses/ENCE361/StellarisWare/utils" --include_path="P:/courses/ENCE361/StellarisWare/boards/ek-lm3s1968" --include_path="P:/courses/ENCE361/StellarisWare" --diag_warning=225 --display_error_number --diag_wrap=off --preproc_with_compile --preproc_dependency="circBuf.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

isqrt.obj: P:/courses/ENCE361/StellarisWare/utils/isqrt.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"c:/ti/ccsv6/tools/compiler/arm_5.1.6/bin/armcl" -mv7M3 --code_state=16 --abi=eabi -me -g --include_path="c:/ti/ccsv6/tools/compiler/arm_5.1.6/include" --include_path="P:/courses/ENCE361/StellarisWare/utils" --include_path="P:/courses/ENCE361/StellarisWare/boards/ek-lm3s1968" --include_path="P:/courses/ENCE361/StellarisWare" --diag_warning=225 --display_error_number --diag_wrap=off --preproc_with_compile --preproc_dependency="isqrt.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

rit128x96x4.obj: P:/courses/ENCE361/StellarisWare/boards/ek-lm3s1968/drivers/rit128x96x4.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"c:/ti/ccsv6/tools/compiler/arm_5.1.6/bin/armcl" -mv7M3 --code_state=16 --abi=eabi -me -g --include_path="c:/ti/ccsv6/tools/compiler/arm_5.1.6/include" --include_path="P:/courses/ENCE361/StellarisWare/utils" --include_path="P:/courses/ENCE361/StellarisWare/boards/ek-lm3s1968" --include_path="P:/courses/ENCE361/StellarisWare" --diag_warning=225 --display_error_number --diag_wrap=off --preproc_with_compile --preproc_dependency="rit128x96x4.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

startup_ccs.obj: ../startup_ccs.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"c:/ti/ccsv6/tools/compiler/arm_5.1.6/bin/armcl" -mv7M3 --code_state=16 --abi=eabi -me -g --include_path="c:/ti/ccsv6/tools/compiler/arm_5.1.6/include" --include_path="P:/courses/ENCE361/StellarisWare/utils" --include_path="P:/courses/ENCE361/StellarisWare/boards/ek-lm3s1968" --include_path="P:/courses/ENCE361/StellarisWare" --diag_warning=225 --display_error_number --diag_wrap=off --preproc_with_compile --preproc_dependency="startup_ccs.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

ustdlib.obj: P:/courses/ENCE361/StellarisWare/utils/ustdlib.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"c:/ti/ccsv6/tools/compiler/arm_5.1.6/bin/armcl" -mv7M3 --code_state=16 --abi=eabi -me -g --include_path="c:/ti/ccsv6/tools/compiler/arm_5.1.6/include" --include_path="P:/courses/ENCE361/StellarisWare/utils" --include_path="P:/courses/ENCE361/StellarisWare/boards/ek-lm3s1968" --include_path="P:/courses/ENCE361/StellarisWare" --diag_warning=225 --display_error_number --diag_wrap=off --preproc_with_compile --preproc_dependency="ustdlib.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


