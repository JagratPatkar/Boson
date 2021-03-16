<p align="center">
  <img  height="300" width="700" src="https://github.com/JagratPatkar/Boson/blob/main/img/Boson%20Logo.svg"/>
</p>



#
__Boson__ is envisioned to be a multi-paradigm language, with a **Static and Strong Type System**. __Boson Compiler__ currently in its initial stage has a Type System and implements *Structured Programming Paradigm*, more on language reference can be found in the docs [here](https://boson.jagrat.com/). The compiler is in active development and will evolve with more paradigms incrementally.


# Examples 



## Binary Search Algorithm

```rust
...
int a[10] = [55,98,67,23,1088,65,44,34,1,7];

fn binarySearchA(int l,int t,int r) int {
    int m;
    if(l <= r){
        m = (l + r) / 2;
        if(a[m] < t){  return binarySearchA(m+1,t,r);  }
        else {
            if(a[m] > t){  return binarySearchA(l,t,m-1);  }
            else{ return m; }
        }
    }
    return 101;
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

#Native C utility function calls
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
