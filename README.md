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

'''rust
consume fn printint(int n) void;


int a[10] = [55,98,67,23,1088,65,44,34,1,7];

fn sortA() void {
    int max;
    int index;
    int temp;
    for(int i = 9; i >= 0; i = i - 1){
        max = a[0];
        index = 0;
        for(int j = 1; j <= i; j = j + 1){
            if(max < a[j]){
                max = a[j];
                index = j;
            }else{}
        }
        temp = a[i];
        a[i] = max;
        a[index] = temp;
    }
    return;
}

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

fn start() void {
    sortA();
    binarySearchA(0,44,9);
    printint(index);
    return;
}
'''


## Ascii Art

<p align="center">
  <img  height="300" width="400" src="https://github.com/JagratPatkar/Boson/blob/main/img/asciiart.png"/>
</p>
