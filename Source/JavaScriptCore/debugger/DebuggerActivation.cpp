/*
 * Copyright (C) 2008, 2009 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#include "config.h"
#include "DebuggerActivation.h"

#include "JSActivation.h"

namespace JSC {

DebuggerActivation::DebuggerActivation(JSGlobalData& globalData)
    : JSNonFinalObject(globalData, globalData.debuggerActivationStructure.get())
{
}

void DebuggerActivation::finishCreation(JSGlobalData& globalData, JSObject* activation)
{
    Base::finishCreation(globalData);
    ASSERT(activation);
    ASSERT(activation->isActivationObject());
    m_activation.set(globalData, this, static_cast<JSActivation*>(activation));
}

void DebuggerActivation::visitChildren(JSCell* cell, SlotVisitor& visitor)
{
    DebuggerActivation* thisObject = static_cast<DebuggerActivation*>(cell);
    ASSERT_GC_OBJECT_INHERITS(thisObject, &s_info);
    COMPILE_ASSERT(StructureFlags & OverridesVisitChildren, OverridesVisitChildrenWithoutSettingFlag);
    ASSERT(thisObject->structure()->typeInfo().overridesVisitChildren());
    JSObject::visitChildren(thisObject, visitor);

    if (thisObject->m_activation)
        visitor.append(&thisObject->m_activation);
}

UString DebuggerActivation::className() const
{
    return m_activation->className();
}

bool DebuggerActivation::getOwnPropertySlot(JSCell* cell, ExecState* exec, const Identifier& propertyName, PropertySlot& slot)
{
    DebuggerActivation* thisObject = static_cast<DebuggerActivation*>(cell);
    return thisObject->m_activation->methodTable()->getOwnPropertySlot(thisObject->m_activation.get(), exec, propertyName, slot);
}

void DebuggerActivation::put(JSCell* cell, ExecState* exec, const Identifier& propertyName, JSValue value, PutPropertySlot& slot)
{
    DebuggerActivation* thisObject = static_cast<DebuggerActivation*>(cell);
    thisObject->m_activation->methodTable()->put(thisObject->m_activation.get(), exec, propertyName, value, slot);
}

void DebuggerActivation::putWithAttributes(ExecState* exec, const Identifier& propertyName, JSValue value, unsigned attributes)
{
    m_activation->putWithAttributes(exec, propertyName, value, attributes);
}

bool DebuggerActivation::deleteProperty(JSCell* cell, ExecState* exec, const Identifier& propertyName)
{
    DebuggerActivation* thisObject = static_cast<DebuggerActivation*>(cell);
    return thisObject->m_activation->methodTable()->deleteProperty(thisObject->m_activation.get(), exec, propertyName);
}

void DebuggerActivation::getOwnPropertyNames(JSObject* object, ExecState* exec, PropertyNameArray& propertyNames, EnumerationMode mode)
{
    DebuggerActivation* thisObject = static_cast<DebuggerActivation*>(object);
    thisObject->m_activation->getPropertyNames(exec, propertyNames, mode);
}

bool DebuggerActivation::getOwnPropertyDescriptor(ExecState* exec, const Identifier& propertyName, PropertyDescriptor& descriptor)
{
    return m_activation->getOwnPropertyDescriptor(exec, propertyName, descriptor);
}

void DebuggerActivation::defineGetter(JSObject* object, ExecState* exec, const Identifier& propertyName, JSObject* getterFunction, unsigned attributes)
{
    DebuggerActivation* thisObject = static_cast<DebuggerActivation*>(object);
    thisObject->m_activation->methodTable()->defineGetter(thisObject->m_activation.get(), exec, propertyName, getterFunction, attributes);
}

void DebuggerActivation::defineSetter(JSObject* object, ExecState* exec, const Identifier& propertyName, JSObject* setterFunction, unsigned attributes)
{
    DebuggerActivation* thisObject = static_cast<DebuggerActivation*>(object);
    thisObject->m_activation->methodTable()->defineSetter(thisObject->m_activation.get(), exec, propertyName, setterFunction, attributes);
}

} // namespace JSC
