class PrintJob;       // forward 声明 参见Effective C++条款34
class Printer
{
public:
    void submitJob(const PrintJob& job);
    void reset();
    void performSelfTest();
    ...
    friend Printer& thePrinter();
private:
    Printer();
    Printer(const Printer& rhs);
    ...
};
Printer& thePrinter()
{
    static Printer p;                          // 单个打印机对象
    return p;
}

