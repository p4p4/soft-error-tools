// this file uses jinja templates for code generation, see http://jinja.pocoo.org
`define WORD_WIDTH 4  // the bit width of the input, adders, and output signals
`define LAYERS 3      // defines how often the adder is cascaded

module top(
    input_vector,   // the input vector, WORD_WIDTH broad
    hold_signals,
    sum,            // the final result, WORD_WIDTH broad
    clk,
    Err_out_Final   // the alarm output
    );
    
    input   clk;
    input   [4-1:0] input_vector;
    input   [3-1:0] hold_signals;
    output  [4-1:0] sum;
    output  Err_out_Final;

    wire [4-1:0] b_out_0 = input_vector;
    wire [4-1:0] result_0 = input_vector;
    wire [3-1:0] error_signals;

    // -- begin unrolled layers --

    wire [4-1:0] b_out_1;
    wire [4-1:0] result_1;


    //full_adder_nbit adder2(clk, result_0, b_out_0, result_1, b_out_1, hold_signals[0], error_signals[0]);
    wire [4-1:0] sum_1; 
    assign sum_1 = result_0 + b_out_0;



    // parity_protected_memory result(clk, res, result_1, hold_signals[0], result_1_latches_invalid);
    reg [4-1:0] memory_sum_L1;
    reg parity_reg_sum_L1;
    wire parity_in_sum_L1;
    wire parity_out_sum_L1;
    assign parity_in_sum_L1 = sum_1[0] ^ sum_1[1] ^ sum_1[2] ^ sum_1[3];
    assign parity_out_sum_L1 = result_1[0] ^ result_1[1] ^ result_1[2] ^ result_1[3];

    assign result_1 = memory_sum_L1;

    initial begin
        memory_sum_L1 = 1'b0;
        parity_reg_sum_L1 = 0;
    end

    always @(posedge clk) begin
        if (~hold_signals[0]) begin
            memory_sum_L1 = sum_1;
            parity_reg_sum_L1 = parity_in_sum_L1;
        end
    end
    // END parity_protected_memory

    //parity_protected_memory input_delay(clk, b_out_0, b_out_1, hold_signals[0], in_latches_invalid);
    reg [4-1:0] memory_delay_L1;
    reg parity_reg_delay_L1;

    wire parity_in_d_L1;
    wire parity_out_d_L1;

    assign parity_in_d_L1 = b_out_0[0] ^ b_out_0[1] ^ b_out_0[2] ^ b_out_0[3];
    assign parity_out_d_L1 = b_out_1[0] ^ b_out_1[1] ^ b_out_1[2] ^ b_out_1[3];

    assign b_out_1 = memory_delay_L1;

    initial begin
        memory_delay_L1 = 1'b0;
        parity_reg_delay_L1 = 0;
    end

    always @(posedge clk) begin
        if (~hold_signals[0]) begin
            memory_delay_L1 = b_out_0;
            parity_reg_delay_L1 = parity_in_d_L1;
        end
    end

    assign error_signals[0] = (parity_reg_delay_L1 ^ parity_out_d_L1) | (parity_reg_sum_L1 ^ parity_out_sum_L1);

    wire [4-1:0] b_out_2;
    wire [4-1:0] result_2;


    //full_adder_nbit adder2(clk, result_1, b_out_1, result_2, b_out_2, hold_signals[1], error_signals[1]);
    wire [4-1:0] sum_2; 
    assign sum_2 = result_1 + b_out_1;



    // parity_protected_memory result(clk, res, result_2, hold_signals[1], result_2_latches_invalid);
    reg [4-1:0] memory_sum_L2;
    reg parity_reg_sum_L2;
    wire parity_in_sum_L2;
    wire parity_out_sum_L2;
    assign parity_in_sum_L2 = sum_2[0] ^ sum_2[1] ^ sum_2[2] ^ sum_2[3];
    assign parity_out_sum_L2 = result_2[0] ^ result_2[1] ^ result_2[2] ^ result_2[3];

    assign result_2 = memory_sum_L2;

    initial begin
        memory_sum_L2 = 1'b0;
        parity_reg_sum_L2 = 0;
    end

    always @(posedge clk) begin
        if (~hold_signals[1]) begin
            memory_sum_L2 = sum_2;
            parity_reg_sum_L2 = parity_in_sum_L2;
        end
    end
    // END parity_protected_memory

    //parity_protected_memory input_delay(clk, b_out_1, b_out_2, hold_signals[1], in_latches_invalid);
    reg [4-1:0] memory_delay_L2;
    reg parity_reg_delay_L2;

    wire parity_in_d_L2;
    wire parity_out_d_L2;

    assign parity_in_d_L2 = b_out_1[0] ^ b_out_1[1] ^ b_out_1[2] ^ b_out_1[3];
    assign parity_out_d_L2 = b_out_2[0] ^ b_out_2[1] ^ b_out_2[2] ^ b_out_2[3];

    assign b_out_2 = memory_delay_L2;

    initial begin
        memory_delay_L2 = 1'b0;
        parity_reg_delay_L2 = 0;
    end

    always @(posedge clk) begin
        if (~hold_signals[1]) begin
            memory_delay_L2 = b_out_1;
            parity_reg_delay_L2 = parity_in_d_L2;
        end
    end

    assign error_signals[1] = (parity_reg_delay_L2 ^ parity_out_d_L2) | (parity_reg_sum_L2 ^ parity_out_sum_L2);

    wire [4-1:0] b_out_3;
    wire [4-1:0] result_3;


    //full_adder_nbit adder2(clk, result_2, b_out_2, result_3, b_out_3, hold_signals[2], error_signals[2]);
    wire [4-1:0] sum_3; 
    assign sum_3 = result_2 + b_out_2;



    // parity_protected_memory result(clk, res, result_3, hold_signals[2], result_3_latches_invalid);
    reg [4-1:0] memory_sum_L3;
    reg parity_reg_sum_L3;
    wire parity_in_sum_L3;
    wire parity_out_sum_L3;
    assign parity_in_sum_L3 = sum_3[0] ^ sum_3[1] ^ sum_3[2] ^ sum_3[3];
    assign parity_out_sum_L3 = result_3[0] ^ result_3[1] ^ result_3[2] ^ result_3[3];

    assign result_3 = memory_sum_L3;

    initial begin
        memory_sum_L3 = 1'b0;
        parity_reg_sum_L3 = 0;
    end

    always @(posedge clk) begin
        if (~hold_signals[2]) begin
            memory_sum_L3 = sum_3;
            parity_reg_sum_L3 = parity_in_sum_L3;
        end
    end
    // END parity_protected_memory

    //parity_protected_memory input_delay(clk, b_out_2, b_out_3, hold_signals[2], in_latches_invalid);
    reg [4-1:0] memory_delay_L3;
    reg parity_reg_delay_L3;

    wire parity_in_d_L3;
    wire parity_out_d_L3;

    assign parity_in_d_L3 = b_out_2[0] ^ b_out_2[1] ^ b_out_2[2] ^ b_out_2[3];
    assign parity_out_d_L3 = b_out_3[0] ^ b_out_3[1] ^ b_out_3[2] ^ b_out_3[3];

    assign b_out_3 = memory_delay_L3;

    initial begin
        memory_delay_L3 = 1'b0;
        parity_reg_delay_L3 = 0;
    end

    always @(posedge clk) begin
        if (~hold_signals[2]) begin
            memory_delay_L3 = b_out_2;
            parity_reg_delay_L3 = parity_in_d_L3;
        end
    end

    assign error_signals[2] = (parity_reg_delay_L3 ^ parity_out_d_L3) | (parity_reg_sum_L3 ^ parity_out_sum_L3);

    // -- end unrolled layers --

    //checker myChecker(hold_signals, error_signals, Err_out_Final);
    // -- begin unrolled checker --

    // error in layer 0:
    wire err_in_L0 = error_signals[0]  & ~hold_signals[1] ;

    // error in layer 1:
    wire err_in_L1 = error_signals[1]  & ~hold_signals[2] ;

    // error in layer 2:
    wire err_in_L2 = error_signals[2];


    assign Err_out_Final = err_in_L0 | err_in_L1 | err_in_L2;
    // -- end unrolled checker --


    assign sum = result_3;

endmodule // top module
