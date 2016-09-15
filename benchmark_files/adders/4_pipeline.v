module top(
    cout,
    in_a,
    in_b,
    in_c,
    sum
    );
    parameter   reg_size = 4;
    
    input   cin;
    input   [reg_size-1:0] in_a;
    input   [reg_size-1:0] in_b;
    input   [reg_size-1:0] in_c;
    output  [reg_size-1:0] sum;
    output  cout;

    wire [reg_size-1:0] sum_intermediate;
    wire c_intermediate;

    full_adder_nbit adder1(cin, c_intermediate, in_a, in_b, sum_intermediate);
    full_adder_nbit adder2(c_intermediate, cout, sum_intermediate, in_c, sum);

//wire [reg_size-1:0] intermediate;
//full_adder_nbit inst2(cin, cout2, sum, in_b, intermediate);
//full_adder_nbit inst2(cin, cout2, intermediate, in_b, sum2);

endmodule

module full_adder_nbit(
    cin,
    cout,
    in_a,
    in_b,
    sum
    );
    
    parameter   reg_size = 4;
    
    input   cin;
    input   [reg_size-1:0] in_a;
    input   [reg_size-1:0] in_b;
    output  [reg_size-1:0] sum;
    output  cout;
    
    assign   {cout,sum} = in_a + in_b + cin;
    
endmodule
