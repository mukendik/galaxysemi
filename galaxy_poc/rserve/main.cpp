/* -------------------------------------------------------------------------- */
/* rserve poc */
/* -------------------------------------------------------------------------- */
#include <fstream>
#include <sstream>
#include <err.h>
#define MAIN
#include <sisocks.h>
#include <Rconnection.h>

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        std::cout << "0 0" << std::endl;
        errx(EXIT_SUCCESS, "Usage: %s <sample-file>", argv[0]);
    }

    Rconnection c("/tmp/RS-socket", -1);
    int r = c.connect();
    if (r)
    {
        std::cout << "0 0" << std::endl;
        errx(EXIT_FAILURE, "error: connect (result=%d)", r);
    }

    std::ifstream source;
    source.open(argv[1], std::ios_base::in);
    unsigned int count = 0;
    for (std::string line; std::getline(source, line);)
    {
        ++count;
    }
    source.close();

    double* sample = new double[count];
    unsigned int i = 0;
    source.open(argv[1], std::ios_base::in);
    for (std::string line; std::getline(source, line); ++i)
    {
        std::istringstream in(line);
        in >> sample[i];
    }
    source.close();

    Rdouble* rd = new Rdouble(sample, count);
    c.assign("sample", rd);
    Rvector* rv =
        (Rvector*) c.eval("qqnorm(sample, plot.it=FALSE)");
    if (rv == NULL)
    {
        std::cout << "0 0" << std::endl;
        errx(EXIT_FAILURE, "error: qqnorm");
    }
    Rdouble* rx = (Rdouble*) rv->elementAt(0);
    Rdouble* ry = (Rdouble*) rv->elementAt(1);

    double* x = rx->doubleArray();
    double* y = ry->doubleArray();
    Rsize_t n = rx->length();
    for (i = 0; i < n; ++i)
    {
        std::cout << y[i] << " " << x[i] << std::endl;
    }

    delete rv;
    delete rd;

    rd = (Rdouble*) c.eval(
            "probs <- c(0.25, 0.75);"
            "y <- quantile(sample, probs, names=FALSE, type=7, na.rm=TRUE);"
            "x <- qnorm(probs);"
            "slope <- diff(y) / diff(x);"
            "int <- y[1L] - slope * x[1L];"
            "c(int, slope);");
    if (rd == NULL)
    {
        errx(EXIT_FAILURE, "error: qqline");
    }
    double* line = rd->doubleArray();
    for (double d = -3; d < 3; d += 0.01)
    {
        std::cout << line[0] + line[1] * d << " " << d << std::endl;
    }

    delete rd;

    delete[] sample;
    c.shutdown(NULL);
    c.disconnect();

    return 0;
}
