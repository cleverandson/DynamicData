/*
 
    This file is part of DynamicData.

    DynamicData is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Foobar is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 
*/

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
        
        //initialize a ddIndex with index type unsigend long and value type unisgend long
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
            std::cout << "pos " << i << " value " << ddIndex.get(i) << std::endl;
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
        
        //ddIndex.updateIdx(2, 1111);
    
        //the index now looks like this
        // pos 0 -> 10
        // pos 1 -> 9
        // pos 2 -> 1111
    }
};

#endif
