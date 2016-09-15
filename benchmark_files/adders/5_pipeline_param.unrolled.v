module top(
    cout,
    input_vector,
    sum,
    clk
    );

    //reg test;
/*
always begin
  test = input_vector[0];
end
initial begin
  test = 0;
end
*/
    parameter WORD_WIDTH = 4;
    parameter LAYERS = 1;
    
    input   cin;
    input   clk;
    input   [WORD_WIDTH-1:0] input_vector;
    output  [WORD_WIDTH-1:0] sum;
    output  cout;


    //assign cout = test;

    //wire [WORD_WIDTH-1:0] sum_intermediate [LAYERS-1:0];
    //wire carry_outputs [LAYERS-1:0];

    //wire [WORD_WIDTH-1:0] bla [0:LAYERS-1] = 0;
    //assign bla[3:0] = input_vector;

    //wire [WORD_WIDTH-1:0] bla [LAYERS-1:0];
    //`UNPACK_ARRAY(WORD_WIDTH,LAYERS,bla,input_vector)

    //assign cout = cin;
    //assign sum[3] = 1;


/*
    genvar i;
    generate for (i = 0; i < n; i = i + 1) begin
        spielzeug instance();
    end endgenerate
*/

    full_adder_nbit adder2(cin, cout, input_vector, input_vector, sum, clk);

endmodule


/*
module full_adder_with_delay(
    cin,
    cout,
    in_a,
    in_b,
    sum
    );

    parameter WORD_WIDTH = 4;
    reg [WORD_WIDTH-1:0] delay;

input   cin;
    input   [WORD_WIDTH-1:0] in_a;
    input   [WORD_WIDTH-1:0] in_b;
    output  [WORD_WIDTH-1:0] sum;
    output  cout;

    assign sum = delay;



    wire [WORD_WIDTH-1:0] module_output
    full_adder_nbit adder(cin, cout, in_a, in_b, module_output);


    initial delay = 0;
    always delay = module_output;


endmodule
*/

module full_adder_nbit(
    cin,
    cout,
    in_a,
    in_b,
    sum,
    clk
    );

    parameter WORD_WIDTH = 4;
    
    input   cin;
    input   [WORD_WIDTH-1:0] in_a;
    input   [WORD_WIDTH-1:0] in_b;
    input clk;
    output [WORD_WIDTH-1:0] sum;
    output  cout;
    
    wire [WORD_WIDTH-1:0] res;
    wire [WORD_WIDTH-1:0] res2;
    parity_protected_memory mem(clk, res, res2); 
    assign   {cout,res} = in_a + in_b + cin;
    assign sum = res2;
    
endmodule

module parity_protected_memory(clk, in,out);
    parameter WORD_WIDTH = 4;

    reg [WORD_WIDTH-1:0] memory;
    initial memory = 1'b0;
    always @(posedge clk) memory = in;
    input  [WORD_WIDTH-1:0] in;
    input clk;
    output [WORD_WIDTH-1:0] out;

    assign out = memory;

endmodule

module my_dff (q, clk, d);
output q;
input d;
input clk;
  reg reg_q;
  assign q = reg_q; // Blocking style
  always @(posedge clk) begin
    reg_q <= d;
  end
endmodule