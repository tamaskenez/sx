
template<typename T>
class A {
    T t;
public:
    void a(){}
};

template<typename T>
class B : private A<T> {
    public:
    using A<T>::a;
};

void fA(A<int> a){

}

void fb() {
    B<int> b;
    b.a();
}

int main(int argc, char* argv[]) {
    return 0;
}
