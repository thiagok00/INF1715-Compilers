# INF1715 - Compilers
A compiler for the monga language, written for the class INF1715-Compilers at PUC-Rio.

## Monga
http://www.inf.puc-rio.br/~roberto/comp/lang.html
  
  ## Depedencies
    flex bison clang-3.8
  
  ## Usage
  make                  
  ./out < tests/you_test.mmg            
  clang-3.8 -o monga monga.ll              
  ./monga               
