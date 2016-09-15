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

module top (a, b, c) ;
input a, b;
output c;
  my_xor inst (a, b, c);
endmodule
