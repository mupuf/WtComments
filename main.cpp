#include "wtcomments.h"

Wt::WApplication *createApplication(const Wt::WEnvironment& env)
{
    return new WtComments(env);
}

int main(int argc, char **argv)
{
    return Wt::WRun(argc, argv, &createApplication);
}
