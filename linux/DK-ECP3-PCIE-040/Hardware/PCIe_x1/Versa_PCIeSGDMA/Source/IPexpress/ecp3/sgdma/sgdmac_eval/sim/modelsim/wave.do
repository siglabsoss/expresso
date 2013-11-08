onerror {resume}
quietly WaveActivateNextPane {} 0
add wave -noupdate -format Literal -label testnum -radix decimal /sgdmac_test/testnum
add wave -noupdate -format Literal -label test_fail -radix unsigned /sgdmac_test/ctl_rtn
add wave -noupdate -format Logic -radix hexadecimal /sgdmac_test/clk
add wave -noupdate -format Logic -radix hexadecimal /sgdmac_test/rst
add wave -noupdate -divider {master A}
add wave -noupdate -format Literal -radix hexadecimal /sgdmac_test/ma_cyc
add wave -noupdate -format Literal -radix hexadecimal /sgdmac_test/ma_stb
add wave -noupdate -format Literal -radix hexadecimal /sgdmac_test/ma_sel
add wave -noupdate -format Literal -radix hexadecimal /sgdmac_test/ma_cti
add wave -noupdate -format Literal -radix hexadecimal /sgdmac_test/ma_we
add wave -noupdate -format Literal -radix hexadecimal /sgdmac_test/ma_adr
add wave -noupdate -format Literal -radix hexadecimal /sgdmac_test/ma_wdat
add wave -noupdate -format Literal -radix hexadecimal /sgdmac_test/ma_rdat
add wave -noupdate -format Literal -radix hexadecimal /sgdmac_test/ma_ack
add wave -noupdate -format Literal -radix symbolic /sgdmac_test/ma_eod
add wave -noupdate -divider {master B}
add wave -noupdate -format Literal -radix hexadecimal /sgdmac_test/mb_cyc
add wave -noupdate -format Literal -radix hexadecimal /sgdmac_test/mb_stb
add wave -noupdate -format Literal -radix hexadecimal /sgdmac_test/mb_sel
add wave -noupdate -format Literal -radix hexadecimal /sgdmac_test/mb_cti
add wave -noupdate -format Literal -radix hexadecimal /sgdmac_test/mb_we
add wave -noupdate -format Literal -radix hexadecimal /sgdmac_test/mb_adr
add wave -noupdate -format Literal -radix hexadecimal /sgdmac_test/mb_wdat
add wave -noupdate -format Literal -radix hexadecimal /sgdmac_test/mb_rdat
add wave -noupdate -format Literal -radix hexadecimal /sgdmac_test/mb_ack
add wave -noupdate -divider {bd ram intf}
add wave -noupdate -format Literal -radix hexadecimal /sgdmac_test/dmac/bd_raddr
add wave -noupdate -format Literal -radix hexadecimal /sgdmac_test/dmac/bd_rdat
add wave -noupdate -format Logic -radix hexadecimal /sgdmac_test/dmac/bd_re
add wave -noupdate -format Logic -radix hexadecimal /sgdmac_test/dmac/bd_rval
add wave -noupdate -format Literal -radix hexadecimal /sgdmac_test/dmac/bd_waddr
add wave -noupdate -format Literal -radix hexadecimal /sgdmac_test/dmac/bd_wdat
add wave -noupdate -format Logic -radix hexadecimal /sgdmac_test/dmac/bd_we
add wave -noupdate -divider {packet buffer}
add wave -noupdate -format Literal -radix hexadecimal /sgdmac_test/dmac/pb_raddr
add wave -noupdate -format Literal -radix hexadecimal /sgdmac_test/dmac/pb_rdat
add wave -noupdate -format Logic -radix hexadecimal /sgdmac_test/dmac/pb_read
add wave -noupdate -format Logic -radix hexadecimal /sgdmac_test/dmac/pb_rval
add wave -noupdate -format Literal -radix hexadecimal /sgdmac_test/dmac/pb_waddr
add wave -noupdate -format Literal -radix hexadecimal /sgdmac_test/dmac/pb_wdat
add wave -noupdate -format Logic -radix hexadecimal /sgdmac_test/dmac/pb_write
add wave -noupdate -divider <NULL>
add wave -noupdate -format Literal -radix hexadecimal /sgdmac_test/subchan
add wave -noupdate -format Literal -radix hexadecimal /sgdmac_test/dma_req
add wave -noupdate -format Literal -radix hexadecimal /sgdmac_test/dma_ack
add wave -noupdate -format Literal -radix hexadecimal /sgdmac_test/eventx
add wave -noupdate -format Literal -radix hexadecimal /sgdmac_test/errorx
TreeUpdate [SetDefaultTree]
WaveRestoreCursors {{Cursor 1} {289854 ps} 0}
configure wave -namecolwidth 289
configure wave -valuecolwidth 110
configure wave -justifyvalue left
configure wave -signalnamewidth 0
configure wave -snapdistance 10
configure wave -datasetprefix 0
configure wave -rowmargin 4
configure wave -childrowmargin 2
configure wave -gridoffset 0
configure wave -gridperiod 1
configure wave -griddelta 40
configure wave -timeline 0
update
WaveRestoreZoom {463598 ps} {468074 ps}
