class PrintJob;       // forward ���� �μ�Effective C++����34
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
    static Printer p;                          // ������ӡ������
    return p;
}

