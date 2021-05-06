#include 	<avr/io.h>

#define		TEMPL	R24
#define		TEMPH	R25

	.align	2
	.lcomm 	rtc_seconds, 	4
	.lcomm 	rtc_ticks,		1

	.section .text

	.global rtc_begin
	.global rtc_getTicks
	.global rtc_getSeconds
	.global	rtc_setSeconds
	
	.global SIG_OUTPUT_COMPARE2
	
rtc_assr_wait:
	in		TEMPL, _SFR_IO_ADDR(ASSR)				// wait for any previous changes to TCCR2 and OCR2 to take effect
	andi	TEMPL, ((1 << TCN2UB) | (1 << OCR2UB) | (1 << TCR2UB))
	brne	rtc_assr_wait
	ret

rtc_begin:
	rcall	rtc_assr_wait
	
	in		TEMPL, _SFR_IO_ADDR(TIMSK)
	andi	TEMPL, ~((1 << OCIE2) | (1 << TOIE2))	// disable any existing timer 2 interrupts
	out		_SFR_IO_ADDR(TIMSK), TEMPL

	ldi		TEMPL, (1 << AS2)						// select asynchronous operation of timer2 (32,768kHz)
	out		_SFR_IO_ADDR(ASSR), TEMPL	


	ldi		TEMPL, (1 << CS20) | (1 << WGM21)		// start timer 2 with no prescaling in CTC mode
	out		_SFR_IO_ADDR(TCCR2), TEMPL
	
	rcall	rtc_assr_wait
	

	ldi		TEMPL, 127								// set timer2 compare value for divide by 128
	out		_SFR_IO_ADDR(OCR2), TEMPL
	
	rcall	rtc_assr_wait

	ldi		TEMPL, (1 << OCF2) | (1 << TOV2)		// clear interrupt flags
	out		_SFR_IO_ADDR(TIFR), TEMPL		
	
	in		TEMPL, _SFR_IO_ADDR(TIMSK)				// enable interrupts
	ori		TEMPL, (1 << OCIE2)		
	out		_SFR_IO_ADDR(TIMSK), TEMPL
	sei	
	ret
	
rtc_getTicks:
	lds		R24, rtc_ticks
	clr		R25
	ret
	
	
rtc_getSeconds:
	ldi		ZL, lo8(rtc_seconds)
	ldi		ZH, hi8(rtc_seconds)
	cli
	ld		R22, Z+
	ld		R23, Z+
	ld		R24, Z+
	ld		R25, Z+
	sei
	ret


rtc_setSeconds:
	ldi		ZL, lo8(rtc_seconds)
	ldi		ZH, hi8(rtc_seconds)
	cli
	st		Z+, R22
	st		Z+, R23
	st		Z+, R24
	st		Z+, R25
	sei
	ret	


SIG_OUTPUT_COMPARE2:
	push	TEMPL
 	push	TEMPH
	push	ZL
	push	ZH
	in		TEMPL, _SFR_IO_ADDR(SREG)
	push	TEMPL
	
	sbi		_SFR_IO_ADDR(PORTD), 7
	sbi		_SFR_IO_ADDR(DDRD), 7
	

periodicUpdate:
	ldi		ZL, lo8(rtc_seconds)
	ldi		ZH, hi8(rtc_seconds)

tickUpdate:
	ldd		TEMPL, Z + 4
	ld		TEMPH, Z
	adiw 	TEMPL, 1
	std		Z + 4, TEMPL
	st		Z, TEMPH
	clr		TEMPH
	ldd		TEMPL, Z + 1
	adc		TEMPL, TEMPH
	std		Z + 1, TEMPL
	ldd		TEMPL, Z + 2
	adc		TEMPL, TEMPH
	std		Z + 2, TEMPL
	ldd		TEMPL, Z + 3
	adc		TEMPL, TEMPH
	std		Z + 3, TEMPL
	
	cbi		_SFR_IO_ADDR(PORTD), 7

periodicUpdateDone:
	pop		TEMPL
	out		_SFR_IO_ADDR(SREG), TEMPL
	pop		ZH
	pop		ZL
	pop		TEMPH
	pop		TEMPL
	reti
