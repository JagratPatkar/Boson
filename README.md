<p align="center">
  <img  height="300" width="700" src="https://github.com/JagratPatkar/Boson/blob/main/img/Boson%20Logo.svg"/>
</p>



#
A General Purpose **Programming Language** on **LLVM**. __Boson__ is envisioned to be a multi-paradigm
language, with a *Static* and *Strong Type System*. __Boson Compiler__ currently in its initial
stage has a Type System and the Idioms of *Structured Paradigm* implemented, the list of 
features have been described below. The compiler is in active development and will evolve with 
more paradigms incrementally.




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

#### Code Snippet

```rust


consume fn printchar(int d) void;
consume fn printint(int n) void;
consume fn nextLine() void;


fn printArt() void {

  ...

   for(;i < 9; i = i + 1){
        nextLine();
        printLine(limit1-1,32);
        printchar(124);
        printLine(limit2,32);
        printchar(124);
        printLine(limit1-1,32);
    }

  ...

}

```
