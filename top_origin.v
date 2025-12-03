//***********************************************************************
// top.v
// Top level system including MIPS, memory, and I/Os
//
// 2013-07-04   Created (by matutani)
// 2013-10-07   Byte enable is added (by matutani)
// 2016-06-03   Target is changed from Spartan-3AN to Zynq-7000 (by matutani)
// 2019-08-30   100msec timer is added (by matutani)
// 2024-07-21   SPI output driver is added (by matutani)
//***********************************************************************
`timescale 1ns/1ps
module fpga_top (
	input			clk_125mhz,
	input		[3:0]	sw,
	input		[3:0]	btn,
	output 	reg	[3:0]	led,
	output		[7:0]	lcd,
	output	reg	[7:0]	ioa,
	output	reg	[7:0]	iob
);
wire	[31:0]	pc, instr, readdata, readdata0, readdata1, writedata, dataadr;
wire	[3:0]	byteen;
wire		reset;
wire		memwrite, memtoregM, swc, cs0, cs1, cs2, cs3, cs4, cs5, irq;
reg		clk_62p5mhz;

/* Reset when two buttons are pushed */
assign	reset	= btn[0] & btn[1];

/* 62.5MHz clock */
always @ (posedge clk_125mhz)
	if (reset)	clk_62p5mhz	<= 1;
	else		clk_62p5mhz	<= ~clk_62p5mhz;

/* CPU module (@62.5MHz) */
mips mips (clk_62p5mhz, reset, pc, instr, {7'b0000000, irq}, memwrite, 
	memtoregM, swc, byteen, dataadr, writedata, readdata, 1'b1, 1'b1);

/* Memory(cs0), Switch(cs1), LED(cs2), LCD(cs3), and more ... */
assign	cs0	= dataadr <  32'hff00;
assign	cs1	= dataadr == 32'hff04;
assign	cs2	= dataadr == 32'hff08;

assign	readdata	= cs0 ? readdata0 : cs1 ? readdata1 : 0;

/* Memory module (@125MHz) */
mem mem (clk_125mhz, reset, cs0 & memwrite, pc[15:2], dataadr[15:2], instr, 
		readdata0, writedata, byteen);

/* Timer module (@62.5MHz) */
timer timer (clk_62p5mhz, reset, irq);

/* cs1 */
assign	readdata1	= {24'h0, btn, sw};
/* cs2 */
always @ (posedge clk_62p5mhz or posedge reset)
	if (reset)			led	<= 0;
	else if (cs2 && memwrite)	led	<= writedata[3:0];

endmodule

//***********************************************************************
// 100msec timer for 62.5MHz clock
//
// 2019-08-30 Created (by matutani)
//***********************************************************************
module timer (
	input			clk, reset,
	output			irq
);
reg	[22:0]	counter;

assign	irq = (counter == 23'd6250000);

always @ (posedge clk or posedge reset)
	if (reset) 			counter	<= 0;
	else if (counter < 23'd6250000)	counter	<= counter + 1;
	else 				counter	<= 0;
endmodule

//***********************************************************************
// Memory (32bit x 16384word) with synchronous read ports for BlockRAM
//
// 2013-07-04   Created (by matutani)
// 2013-10-07   Byte enable is added (by matutani)
// 2016-06-03   Memory size is changed from 8192 to 16384 words (by matutani)
//***********************************************************************
module mem (
	input			clk, reset, memwrite,
	input		[13:0]	instradr, dataadr,
	output	reg	[31:0]	instr,
	output	reg	[31:0]	readdata,
	input		[31:0]	writedata,
	input		[3:0]	byteen
);
reg	[31:0]	RAM [0:16383];	/* Memory size is 16384 words */
wire	[7:0]	byte0, byte1, byte2, byte3;

assign	byte0	= byteen[0] ? writedata[ 7: 0] : readdata[ 7: 0];
assign	byte1	= byteen[1] ? writedata[15: 8] : readdata[15: 8];
assign	byte2	= byteen[2] ? writedata[23:16] : readdata[23:16];
assign	byte3	= byteen[3] ? writedata[31:24] : readdata[31:24];

always @ (posedge clk) begin
	if (memwrite)
		RAM[dataadr]	<= {byte3, byte2, byte1, byte0};
	instr	<= RAM[instradr];
	readdata<= RAM[dataadr];
end

/* Specify your program image file (e.g., program.dat) */
initial $readmemh("program.dat", RAM, 0, 16383);
endmodule

//***********************************************************************
// SPI 8-bit output driver for 62.5MHz clock
//
// 2024-07-21   Created (by matutani)
//***********************************************************************
`define	SPI_WAIT	2'b00
`define	SPI_START	2'b01
`define	SPI_TRANS	2'b10
`define	SPI_STOP	2'b11
`define	SPI_DATA	1'b1
`define	SPI_CMD		1'b0
`define	Enable_		1'b0
`define	Disable_	1'b1
`define	SPI_FREQDIV	25	/* 62.5MHz / 2 / 25 = 1.25MHz */

module spi (
	input clk, input reset, input start, input [9:0] din, output [7:0] dout
);
reg	[1:0]	state;
reg	[7:0]	d_reg;
reg	[2:0]	cnt;	/* 8 */
reg	[4:0]	cnt2;	/* 25 */
reg	cs_, dc_, res_, sdo, sck, pmoden, vccen;

assign	dout = {pmoden, vccen, res_, dc_, sck, 1'b0, sdo, cs_};
always @(posedge clk or posedge reset)
	if (reset) begin
		state	<= `SPI_WAIT;
		d_reg	<= 0;
		cnt	<= 0;
		cnt2	<= 0;
		cs_	<= `Disable_;
		dc_	<= 0;
		res_	<= `Enable_;
		sdo	<= 0;
		sck	<= 0;
		pmoden	<= 0; /* Display power OFF */
		vccen	<= 0; /* Display power OFF */
	end else if (state == `SPI_WAIT) begin
		res_	<= `Disable_;
		sck	<= 1;
		if (start && din[9]) begin
			pmoden	<= 1; /* Display power ON */
			vccen	<= 1; /* Display power ON */
		end else if (start) begin
			state	<= `SPI_START;
			d_reg	<= din[7:0];
			cnt	<= 8;
			cs_	<= `Enable_;
			dc_	<= din[8];
			cnt2	<= 0;
		end
	end else if (state == `SPI_START)
		if (cnt2 == `SPI_FREQDIV - 1) begin
			state	<= `SPI_TRANS;
			cnt2	<= 0;
		end else
			cnt2	<= cnt2 + 1;
	else if (state == `SPI_TRANS)
		if (sck)
			if (cnt2 == `SPI_FREQDIV - 1) begin
				sck	<= 0;
				sdo	<= d_reg[7];
				cnt	<= cnt - 1;
				cnt2	<= 0;
			end else
				cnt2	<= cnt2 + 1;
		else
			if (cnt2 == `SPI_FREQDIV - 1) begin
				sck	<= 1;
				d_reg	<= {d_reg[6:0], 1'b0};
				cnt2	<= 0;
				if (cnt == 0)
					state	<= `SPI_STOP;
			end else
				cnt2	<= cnt2 + 1;
	else if (state == `SPI_STOP)
		if (cnt2 == `SPI_FREQDIV - 1) begin
			state	<= `SPI_WAIT;
			cs_	<= `Disable_;
			cnt2	<= 0;
		end else
			cnt2	<= cnt2 + 1;
endmodule
