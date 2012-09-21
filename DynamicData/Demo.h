//
//  Demo.h
//  DynamicData
//
//  Created by mich2 on 9/21/12.
//  Copyright (c) 2012 -. All rights reserved.
//

#ifndef DynamicData_Demo_h
#define DynamicData_Demo_h

class Demo
{
public:
    
    /*
    DDIndex interface
    
    void deleteIdx(IdxType idx)
     
    void insertIdx(IdxType idx, YType yValue)
     
    void updateIdx(IdxType idx, YType yValue)
     
    YType get(IdxType idx, bool& succeeded)
    
     
    IdxType ist eine zahl ohne vorzeichen zb unsigned long
    
    YType ist der resultat typ zb eine kleine klasse oder zahl
    
     
    ddIndex ist persistent
    alle daten werden relativ zur applikation in einem folder data gespeichert.
     
    dem konstruktor werden zahlenwerte übergeben, mit welchen die files im data folder identifiziert werden.
    DDIndex(size_t scopeVal, size_t idVal1, size_t idVal2, size_t idVal3)
    */
    
    static void demo()
    {
        //uncomment to remove the persisted data.
        system("rm -r data");
        
        //initialize a ddIndex
        DDIndex<unsigned long, unsigned long> ddIndex(1, 0, 1, 2);
        
        //check if it allready has cached data included.
        bool hasPersistData = ddIndex.size() > 0;
        
        //if there is no persistent data we insert some.
        if (!hasPersistData)
        {
            persistData(ddIndex);
        }
        
        //on continuous runs of the program the same values on the same positions will be printed out because the
        //values have been persistently saved.
        for (int i=0; i<ddIndex.size(); i++)
        {
            bool succeeded;
            std::cout << "pos " << i << " value " << ddIndex.get(i, succeeded) << std::endl;
        }
        
        //the printed result should look like this
        // pos 0 -> 10
        // pos 1 -> 9
        // pos 2 -> 1111
    }
    
    static void persistData(DDIndex<unsigned long, unsigned long>& ddIndex)
    {
        assert(ddIndex.size() == 0);
        
        ddIndex.insertIdx(0, 7);
        ddIndex.insertIdx(0, 8);
        ddIndex.insertIdx(0, 9);
        ddIndex.insertIdx(0, 10);
        
        //the index now looks like this
        //the order is reversed because of the consecutive inserts at pos 0.
        // pos 0 -> 10
        // pos 1 -> 9
        // pos 2 -> 7
        // pos 3 -> 8
        
        ddIndex.deleteIdx(2);
        
        //the index now looks like this
        // pos 0 -> 10
        // pos 1 -> 9
        // pos 2 -> 8
        
        ddIndex.updateIdx(2, 1111);
    
        //the index now looks like this
        // pos 0 -> 10
        // pos 1 -> 9
        // pos 2 -> 1111
    }
};

#endif
