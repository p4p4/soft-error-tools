module my_xor(in_a , in_b , out );
input
  in_a ,
  in_b ;
output
  out ;
//wire
//  \[2] ;
assign
  out  = in_a ^ in_b ;
endmodule

module top (a, b, c, d, e, f, vector ) ;
input a, b, d, e, [4:0]vector;
output c, f;
  my_xor inst (a, b, c);
  my_xor inst (d, e, f);
endmodule
