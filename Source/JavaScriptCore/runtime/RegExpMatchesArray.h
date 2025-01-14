/*
 *  Copyright (C) 2008 Apple Inc. All Rights Reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef RegExpMatchesArray_h
#define RegExpMatchesArray_h

#include "JSArray.h"

namespace JSC {

    class RegExpMatchesArray : public JSArray {
    private:
        RegExpMatchesArray(ExecState*);

    public:
        typedef JSArray Base;

        static RegExpMatchesArray* create(ExecState* exec, RegExpConstructorPrivate* ctorPrivate)
        {
            RegExpMatchesArray* regExp = new (allocateCell<RegExpMatchesArray>(*exec->heap())) RegExpMatchesArray(exec);
            regExp->finishCreation(exec->globalData(), ctorPrivate);
            return regExp;
        }
        virtual ~RegExpMatchesArray();

        static JS_EXPORTDATA const ClassInfo s_info;
        
        static Structure* createStructure(JSGlobalData& globalData, JSGlobalObject* globalObject, JSValue prototype)
        {
            return Structure::create(globalData, globalObject, prototype, TypeInfo(ObjectType, StructureFlags), &s_info);
        }
        
    protected:
        void finishCreation(JSGlobalData&, RegExpConstructorPrivate* data);

    private:
        static bool getOwnPropertySlot(JSCell* cell, ExecState* exec, const Identifier& propertyName, PropertySlot& slot)
        {
            RegExpMatchesArray* thisObject = static_cast<RegExpMatchesArray*>(cell);
            if (thisObject->subclassData())
                thisObject->fillArrayInstance(exec);
            return JSArray::getOwnPropertySlot(thisObject, exec, propertyName, slot);
        }

        static bool getOwnPropertySlotByIndex(JSCell* cell, ExecState* exec, unsigned propertyName, PropertySlot& slot)
        {
            RegExpMatchesArray* thisObject = static_cast<RegExpMatchesArray*>(cell);
            if (thisObject->subclassData())
                thisObject->fillArrayInstance(exec);
            return JSArray::getOwnPropertySlotByIndex(thisObject, exec, propertyName, slot);
        }

        virtual bool getOwnPropertyDescriptor(ExecState* exec, const Identifier& propertyName, PropertyDescriptor& descriptor)
        {
            if (subclassData())
                fillArrayInstance(exec);
            return JSArray::getOwnPropertyDescriptor(exec, propertyName, descriptor);
        }

        static void put(JSCell* cell, ExecState* exec, const Identifier& propertyName, JSValue v, PutPropertySlot& slot)
        {
            RegExpMatchesArray* thisObject = static_cast<RegExpMatchesArray*>(cell);
            if (thisObject->subclassData())
                thisObject->fillArrayInstance(exec);
            JSArray::put(thisObject, exec, propertyName, v, slot);
        }
        
        static void putByIndex(JSCell* cell, ExecState* exec, unsigned propertyName, JSValue v)
        {
            RegExpMatchesArray* thisObject = static_cast<RegExpMatchesArray*>(cell);
            if (thisObject->subclassData())
                thisObject->fillArrayInstance(exec);
            JSArray::putByIndex(thisObject, exec, propertyName, v);
        }

        static bool deleteProperty(JSCell* cell, ExecState* exec, const Identifier& propertyName)
        {
            RegExpMatchesArray* thisObject = static_cast<RegExpMatchesArray*>(cell);
            if (thisObject->subclassData())
                thisObject->fillArrayInstance(exec);
            return JSArray::deleteProperty(thisObject, exec, propertyName);
        }

        static bool deletePropertyByIndex(JSCell* cell, ExecState* exec, unsigned propertyName)
        {
            RegExpMatchesArray* thisObject = static_cast<RegExpMatchesArray*>(cell);
            if (thisObject->subclassData())
                thisObject->fillArrayInstance(exec);
            return JSArray::deletePropertyByIndex(thisObject, exec, propertyName);
        }

        static void getOwnPropertyNames(JSObject* object, ExecState* exec, PropertyNameArray& arr, EnumerationMode mode = ExcludeDontEnumProperties)
        {
            RegExpMatchesArray* thisObject = static_cast<RegExpMatchesArray*>(object);
            if (thisObject->subclassData())
                thisObject->fillArrayInstance(exec);
            JSArray::getOwnPropertyNames(thisObject, exec, arr, mode);
        }

        void fillArrayInstance(ExecState*);
};

}

#endif // RegExpMatchesArray_h
