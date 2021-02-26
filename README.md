<p align="center">
  <img  height="300" width="700" src="https://github.com/JagratPatkar/Boson/blob/main/img/Boson%20Logo.svg"/>
</p>



#
A General Purpose Programming Language on LLVM. Boson is envisioned to be multi-paradigm
language, with a Statically Typed and Strong Type System. It currently has a Type System 
and the idioms of structural paradigm implemented.




# Features

* __Data Types__
  * Int
  * Double
  * Bool
* __Functional Idioms__
   * Functions 
   * Recursion
* __Operators__
  * Arithmetic Operators
  * Logical Operators
  * Relational Operators
* __Composite Types__
  * Array
* __Structured Idioms__
  * For Loop
  * If Else
* __C Native Calls__
  * Consume

# Examples 



## Binary Search Algorithm

```rust
...

int index;

fn binarySearchA(int l,int t,int r) void {
    int m;
    if(l <= r){
        m = (l + r) / 2;
        if(a[m] < t){  binarySearchA(m+1,t,r);  }
        else {
            if(a[m] > t){  binarySearchA(l,t,m-1);  }
            else{ index = m; }
        }
    }else{ }
    return;
}

...

```


## Ascii Art


```
                                ----------------------------------------                                        
                               ' > Hello Boson                          '                                       
                               |                                        |                                       
                               |                                        |                                       
                               |                                        |                                       
                               |                                        |                                       
                               |                                        |                                       
                               |                                        |                                       
                               |                                        |                                       
                               |                                        |                                       
                               |                                        |                                       
                               .                                        .                                       
                                ----------------------------------------                                        
                                                  ||                                                           
                                                  ||                                                           
                                                  ||                                                           
                                              ==========                                                       
                                         ____________________                                                  
                                        / ~ ~ ~ ~ ~ ~ ~ ~ ~ /                                                 
                                       / ~ ~ ~ ~ ~ ~ ~ ~ ~ /                                                
                                      / ~ ~ ~ ~ ~ ~ ~ ~ ~ /                                               
                                     ====================                                               

```