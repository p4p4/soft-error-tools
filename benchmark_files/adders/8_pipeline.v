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
    input   [`WORD_WIDTH-1:0] input_vector;
    input   [`LAYERS-1:0] hold_signals;
    output  [`WORD_WIDTH-1:0] sum;
    output  Err_out_Final;

    wire [`WORD_WIDTH-1:0] b_out_0 = input_vector;
    wire [`WORD_WIDTH-1:0] result_0 = input_vector;
    wire [`LAYERS-1:0] error_signals;

    // -- begin unrolled layers --
{% for idx in range(1,`LAYERS+1) %}
    wire [`WORD_WIDTH-1:0] b_out_{{ idx }};
    wire [`WORD_WIDTH-1:0] result_{{ idx }};


    //full_adder_nbit adder2(clk, result_{{idx - 1}}, b_out_{{idx - 1}}, result_{{idx}}, b_out_{{idx}}, hold_signals[{{idx - 1}}], error_signals[{{idx - 1}}]);
    wire [`WORD_WIDTH-1:0] sum_{{ idx }}; 
    assign sum_{{idx}} = result_{{idx - 1}} + b_out_{{idx - 1}};



    // parity_protected_memory result(clk, res, result_{{idx}}, hold_signals[{{idx - 1}}], result_{{idx}}_latches_invalid);
    reg [`WORD_WIDTH-1:0] memory_sum_L{{idx}};
    reg parity_reg_sum_L{{idx}};
    wire parity_in_sum_L{{idx}};
    wire parity_out_sum_L{{idx}};
    assign parity_in_sum_L{{idx}} = sum_{{ idx }}[0]{% for ibit in range(1,`WORD_WIDTH) %} ^ sum_{{ idx }}[{{ibit}}]{% endfor %};
    assign parity_out_sum_L{{idx}} = result_{{idx}}[0]{% for ibit in range(1,`WORD_WIDTH) %} ^ result_{{idx}}[{{ibit}}]{% endfor %};

    assign result_{{idx}} = memory_sum_L{{idx}};

    initial begin
        memory_sum_L{{idx}} = 1'b0;
        parity_reg_sum_L{{idx}} = 0;
    end

    always @(posedge clk) begin
        if (~hold_signals[{{idx - 1}}]) begin
            memory_sum_L{{idx}} = sum_{{idx}};
            parity_reg_sum_L{{idx}} = parity_in_sum_L{{idx}};
        end
    end
    // END parity_protected_memory

    //parity_protected_memory input_delay(clk, b_out_{{idx - 1}}, b_out_{{idx}}, hold_signals[{{idx - 1}}], in_latches_invalid);
    reg [`WORD_WIDTH-1:0] memory_delay_L{{idx}};
    reg parity_reg_delay_L{{idx}};

    wire parity_in_d_L{{idx}};
    wire parity_out_d_L{{idx}};

    assign parity_in_d_L{{idx}} = b_out_{{idx - 1}}[0]{% for ibit in range(1,`WORD_WIDTH) %} ^ b_out_{{idx - 1}}[{{ibit}}]{% endfor %};
    assign parity_out_d_L{{idx}} = b_out_{{idx}}[0]{% for ibit in range(1,`WORD_WIDTH) %} ^ b_out_{{idx}}[{{ibit}}]{% endfor %};

    assign b_out_{{idx}} = memory_delay_L{{idx}};

    initial begin
        memory_delay_L{{idx}} = 1'b0;
        parity_reg_delay_L{{idx}} = 0;
    end

    always @(posedge clk) begin
        if (~hold_signals[{{idx - 1}}]) begin
            memory_delay_L{{idx}} = b_out_{{idx - 1}};
            parity_reg_delay_L{{idx}} = parity_in_d_L{{idx}};
        end
    end

    assign error_signals[{{idx - 1}}] = (parity_reg_delay_L{{idx}} ^ parity_out_d_L{{idx}}) | (parity_reg_sum_L{{idx}} ^ parity_out_sum_L{{idx}});
{% endfor %}
    // -- end unrolled layers --

    //checker myChecker(hold_signals, error_signals, Err_out_Final);
    // -- begin unrolled checker --
{% for i1 in range(0,`LAYERS) %}
    // error in layer {{i1}}:
    wire err_in_L{{i1}} = error_signals[{{i1}}]{% if i1 is not equalto `LAYERS - 1%}  & ~hold_signals[{{i1+1}}] {% endif %};
{% endfor %}

    assign Err_out_Final = err_in_L0{% for i1 in range(1,`LAYERS) %} | err_in_L{{i1}}{% endfor %};
    // -- end unrolled checker --


    assign sum = result_`LAYERS;

endmodule // top module

