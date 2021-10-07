#include <SBot_Private/SBot_Tools.h>
#include <iostream>
#include <string>
#include <set>
using namespace std;
int main()
{
    size_t test_count = 100000;
    set<string> uuid_set;
    for(size_t i = 0;i < test_count;++i)
    {
        uuid_set.insert(gen_echo());
    }
    if(uuid_set.size() == test_count)
    {
        cout << "uuid test ok";
    }
    else
    {
        cout << "uuid test failed";
    }
    return 0;
}