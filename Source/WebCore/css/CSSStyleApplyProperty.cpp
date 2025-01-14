/*
 * Copyright (C) 2011 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "CSSStyleApplyProperty.h"

#include "CSSCursorImageValue.h"
#include "CSSFlexValue.h"
#include "CSSPrimitiveValueMappings.h"
#include "CSSStyleSelector.h"
#include "CSSValueList.h"
#include "CursorList.h"
#include "Document.h"
#include "Element.h"
#include "Pair.h"
#include "RenderStyle.h"
#include <wtf/StdLibExtras.h>
#include <wtf/UnusedParam.h>

using namespace std;

namespace WebCore {

enum ExpandValueBehavior {SuppressValue = 0, ExpandValue};
template <ExpandValueBehavior expandValue, CSSPropertyID one = CSSPropertyInvalid, CSSPropertyID two = CSSPropertyInvalid, CSSPropertyID three = CSSPropertyInvalid, CSSPropertyID four = CSSPropertyInvalid>
class ApplyPropertyExpanding {
public:

    template <CSSPropertyID id>
    static inline void applyInheritValue(CSSStyleSelector* selector)
    {
        if (id == CSSPropertyInvalid)
            return;

        const CSSStyleApplyProperty& table = CSSStyleApplyProperty::sharedCSSStyleApplyProperty();
        const PropertyHandler& handler = table.propertyHandler(id);
        if (handler.isValid())
            handler.applyInheritValue(selector);
    }

    static void applyInheritValue(CSSStyleSelector* selector)
    {
        applyInheritValue<one>(selector);
        applyInheritValue<two>(selector);
        applyInheritValue<three>(selector);
        applyInheritValue<four>(selector);
    }

    template <CSSPropertyID id>
    static inline void applyInitialValue(CSSStyleSelector* selector)
    {
        if (id == CSSPropertyInvalid)
            return;

        const CSSStyleApplyProperty& table = CSSStyleApplyProperty::sharedCSSStyleApplyProperty();
        const PropertyHandler& handler = table.propertyHandler(id);
        if (handler.isValid())
            handler.applyInitialValue(selector);
    }

    static void applyInitialValue(CSSStyleSelector* selector)
    {
        applyInitialValue<one>(selector);
        applyInitialValue<two>(selector);
        applyInitialValue<three>(selector);
        applyInitialValue<four>(selector);
    }

    template <CSSPropertyID id>
    static inline void applyValue(CSSStyleSelector* selector, CSSValue* value)
    {
        if (id == CSSPropertyInvalid)
            return;

        const CSSStyleApplyProperty& table = CSSStyleApplyProperty::sharedCSSStyleApplyProperty();
        const PropertyHandler& handler = table.propertyHandler(id);
        if (handler.isValid())
            handler.applyValue(selector, value);
    }

    static void applyValue(CSSStyleSelector* selector, CSSValue* value)
    {
        if (!expandValue)
            return;

        applyValue<one>(selector, value);
        applyValue<two>(selector, value);
        applyValue<three>(selector, value);
        applyValue<four>(selector, value);
    }
    static PropertyHandler createHandler() { return PropertyHandler(&applyInheritValue, &applyInitialValue, &applyValue); }
};

template <typename GetterType, GetterType (RenderStyle::*getterFunction)() const, typename SetterType, void (RenderStyle::*setterFunction)(SetterType), typename InitialType, InitialType (*initialFunction)()>
class ApplyPropertyDefaultBase {
public:
    static void setValue(RenderStyle* style, SetterType value) { (style->*setterFunction)(value); }
    static GetterType value(RenderStyle* style) { return (style->*getterFunction)(); }
    static InitialType initial() { return (*initialFunction)(); }
    static void applyInheritValue(CSSStyleSelector* selector) { setValue(selector->style(), value(selector->parentStyle())); }
    static void applyInitialValue(CSSStyleSelector* selector) { setValue(selector->style(), initial()); }
    static void applyValue(CSSStyleSelector*, CSSValue*) { }
    static PropertyHandler createHandler() { return PropertyHandler(&applyInheritValue, &applyInitialValue, &applyValue); }
};

template <typename GetterType, GetterType (RenderStyle::*getterFunction)() const, typename SetterType, void (RenderStyle::*setterFunction)(SetterType), typename InitialType, InitialType (*initialFunction)()>
class ApplyPropertyDefault {
public:
    static void setValue(RenderStyle* style, SetterType value) { (style->*setterFunction)(value); }
    static void applyValue(CSSStyleSelector* selector, CSSValue* value)
    {
        if (value->isPrimitiveValue())
            setValue(selector->style(), *static_cast<CSSPrimitiveValue*>(value));
    }
    static PropertyHandler createHandler()
    {
        PropertyHandler handler = ApplyPropertyDefaultBase<GetterType, getterFunction, SetterType, setterFunction, InitialType, initialFunction>::createHandler();
        return PropertyHandler(handler.inheritFunction(), handler.initialFunction(), &applyValue);
    }
};

template <StyleImage* (RenderStyle::*getterFunction)() const, void (RenderStyle::*setterFunction)(PassRefPtr<StyleImage>), StyleImage* (*initialFunction)(), CSSPropertyID property>
class ApplyPropertyStyleImage {
public:
    static void applyValue(CSSStyleSelector* selector, CSSValue* value) { (selector->style()->*setterFunction)(selector->styleImage(property, value)); }
    static PropertyHandler createHandler()
    {
        PropertyHandler handler = ApplyPropertyDefaultBase<StyleImage*, getterFunction, PassRefPtr<StyleImage>, setterFunction, StyleImage*, initialFunction>::createHandler();
        return PropertyHandler(handler.inheritFunction(), handler.initialFunction(), &applyValue);
    }
};

enum AutoValueType {Number = 0, ComputeLength};
template <typename T, T (RenderStyle::*getterFunction)() const, void (RenderStyle::*setterFunction)(T), bool (RenderStyle::*hasAutoFunction)() const, void (RenderStyle::*setAutoFunction)(), AutoValueType valueType = Number, int autoIdentity = CSSValueAuto>
class ApplyPropertyAuto {
public:
    static void setValue(RenderStyle* style, T value) { (style->*setterFunction)(value); }
    static T value(RenderStyle* style) { return (style->*getterFunction)(); }
    static bool hasAuto(RenderStyle* style) { return (style->*hasAutoFunction)(); }
    static void setAuto(RenderStyle* style) { (style->*setAutoFunction)(); }

    static void applyInheritValue(CSSStyleSelector* selector)
    {
        if (hasAuto(selector->parentStyle()))
            setAuto(selector->style());
        else
            setValue(selector->style(), value(selector->parentStyle()));
    }

    static void applyInitialValue(CSSStyleSelector* selector) { setAuto(selector->style()); }

    static void applyValue(CSSStyleSelector* selector, CSSValue* value)
    {
        if (!value->isPrimitiveValue())
            return;

        CSSPrimitiveValue* primitiveValue = static_cast<CSSPrimitiveValue*>(value);
        if (primitiveValue->getIdent() == autoIdentity)
            setAuto(selector->style());
        else if (valueType == Number)
            setValue(selector->style(), *primitiveValue);
        else if (valueType == ComputeLength)
            setValue(selector->style(), primitiveValue->computeLength<T>(selector->style(), selector->rootElementStyle(), selector->style()->effectiveZoom()));
    }

    static PropertyHandler createHandler() { return PropertyHandler(&applyInheritValue, &applyInitialValue, &applyValue); }
};

enum ColorInherit {NoInheritFromParent = 0, InheritFromParent};
Color defaultInitialColor();
Color defaultInitialColor() { return Color(); }
template <ColorInherit inheritColorFromParent,
          const Color& (RenderStyle::*getterFunction)() const,
          void (RenderStyle::*setterFunction)(const Color&),
          void (RenderStyle::*visitedLinkSetterFunction)(const Color&),
          const Color& (RenderStyle::*defaultFunction)() const,
          Color (*initialFunction)() = &defaultInitialColor>
class ApplyPropertyColor {
public:
    static void applyInheritValue(CSSStyleSelector* selector)
    {
        // Visited link style can never explicitly inherit from parent visited link style so no separate getters are needed.
        const Color& color = (selector->parentStyle()->*getterFunction)();
        applyColorValue(selector, color.isValid() ? color : (selector->parentStyle()->*defaultFunction)());
    }

    static void applyInitialValue(CSSStyleSelector* selector)
    {
        applyColorValue(selector, initialFunction());
    }

    static void applyValue(CSSStyleSelector* selector, CSSValue* value)
    {
        if (!value->isPrimitiveValue())
            return;

        CSSPrimitiveValue* primitiveValue = static_cast<CSSPrimitiveValue*>(value);
        if (inheritColorFromParent && primitiveValue->getIdent() == CSSValueCurrentcolor)
            applyInheritValue(selector);
        else {
            if (selector->applyPropertyToRegularStyle())
                (selector->style()->*setterFunction)(selector->getColorFromPrimitiveValue(primitiveValue, false));
            if (selector->applyPropertyToVisitedLinkStyle())
                (selector->style()->*visitedLinkSetterFunction)(selector->getColorFromPrimitiveValue(primitiveValue, true));
        }
    }

    static void applyColorValue(CSSStyleSelector* selector, const Color& color)
    {
        if (selector->applyPropertyToRegularStyle())
            (selector->style()->*setterFunction)(color);
        if (selector->applyPropertyToVisitedLinkStyle())
            (selector->style()->*visitedLinkSetterFunction)(color);
    }

    static PropertyHandler createHandler() { return PropertyHandler(&applyInheritValue, &applyInitialValue, &applyValue); }
};

template <TextDirection (RenderStyle::*getterFunction)() const, void (RenderStyle::*setterFunction)(TextDirection), TextDirection (*initialFunction)()>
class ApplyPropertyDirection {
public:
    static void applyValue(CSSStyleSelector* selector, CSSValue* value)
    {
        ApplyPropertyDefault<TextDirection, getterFunction, TextDirection, setterFunction, TextDirection, initialFunction>::applyValue(selector, value);
        Element* element = selector->element();
        if (element && selector->element() == element->document()->documentElement())
            element->document()->setDirectionSetOnDocumentElement(true);
    }

    static PropertyHandler createHandler()
    {
        PropertyHandler handler = ApplyPropertyDefault<TextDirection, getterFunction, TextDirection, setterFunction, TextDirection, initialFunction>::createHandler();
        return PropertyHandler(handler.inheritFunction(), handler.initialFunction(), &applyValue);
    }
};

enum LengthAuto { AutoDisabled = 0, AutoEnabled };
enum LengthIntrinsic { IntrinsicDisabled = 0, IntrinsicEnabled };
enum LengthMinIntrinsic { MinIntrinsicDisabled = 0, MinIntrinsicEnabled };
enum LengthNone { NoneDisabled = 0, NoneEnabled };
enum LengthUndefined { UndefinedDisabled = 0, UndefinedEnabled };
enum LengthFlexDirection { FlexDirectionDisabled = 0, FlexWidth, FlexHeight };
template <Length (RenderStyle::*getterFunction)() const,
          void (RenderStyle::*setterFunction)(Length),
          Length (*initialFunction)(),
          LengthAuto autoEnabled = AutoDisabled,
          LengthIntrinsic intrinsicEnabled = IntrinsicDisabled,
          LengthMinIntrinsic minIntrinsicEnabled = MinIntrinsicDisabled,
          LengthNone noneEnabled = NoneDisabled,
          LengthUndefined noneUndefined = UndefinedDisabled,
          LengthFlexDirection flexDirection = FlexDirectionDisabled>
class ApplyPropertyLength {
public:
    static void setValue(RenderStyle* style, Length value) { (style->*setterFunction)(value); }
    static void applyValue(CSSStyleSelector* selector, CSSValue* value)
    {
#if ENABLE(CSS3_FLEXBOX)
        if (!value->isPrimitiveValue()) {
            if (!flexDirection || !value->isFlexValue())
                return;

            CSSFlexValue* flexValue = static_cast<CSSFlexValue*>(value);
            value = flexValue->preferredSize();

            if (flexDirection == FlexWidth) {
                selector->style()->setFlexboxWidthPositiveFlex(flexValue->positiveFlex());
                selector->style()->setFlexboxWidthNegativeFlex(flexValue->negativeFlex());
            } else if (flexDirection == FlexHeight) {
                selector->style()->setFlexboxHeightPositiveFlex(flexValue->positiveFlex());
                selector->style()->setFlexboxHeightNegativeFlex(flexValue->negativeFlex());
            }
        }
#else
        if (!value->isPrimitiveValue())
            return;
#endif

        CSSPrimitiveValue* primitiveValue = static_cast<CSSPrimitiveValue*>(value);
        if (noneEnabled && primitiveValue->getIdent() == CSSValueNone)
            if (noneUndefined)
                setValue(selector->style(), Length(Undefined));
            else
                setValue(selector->style(), Length());
        else if (intrinsicEnabled && primitiveValue->getIdent() == CSSValueIntrinsic)
            setValue(selector->style(), Length(Intrinsic));
        else if (minIntrinsicEnabled && primitiveValue->getIdent() == CSSValueMinIntrinsic)
            setValue(selector->style(), Length(MinIntrinsic));
        else if (autoEnabled && primitiveValue->getIdent() == CSSValueAuto)
            setValue(selector->style(), Length());
        else {
            int type = primitiveValue->primitiveType();
            if (CSSPrimitiveValue::isUnitTypeLength(type)) {
                Length length = primitiveValue->computeLength<Length>(selector->style(), selector->rootElementStyle(), selector->style()->effectiveZoom());
                length.setQuirk(primitiveValue->isQuirkValue());
                setValue(selector->style(), length);
            } else if (type == CSSPrimitiveValue::CSS_PERCENTAGE)
                setValue(selector->style(), Length(primitiveValue->getDoubleValue(), Percent));
        }
    }

    static PropertyHandler createHandler()
    {
        PropertyHandler handler = ApplyPropertyDefaultBase<Length, getterFunction, Length, setterFunction, Length, initialFunction>::createHandler();
        return PropertyHandler(handler.inheritFunction(), handler.initialFunction(), &applyValue);
    }
};

enum StringIdentBehavior { NothingMapsToNull = 0, MapNoneToNull, MapAutoToNull };
template <StringIdentBehavior identBehavior, const AtomicString& (RenderStyle::*getterFunction)() const, void (RenderStyle::*setterFunction)(const AtomicString&), const AtomicString& (*initialFunction)()>
class ApplyPropertyString {
public:
    static void setValue(RenderStyle* style, const AtomicString& value) { (style->*setterFunction)(value); }
    static void applyValue(CSSStyleSelector* selector, CSSValue* value)
    {
        if (!value->isPrimitiveValue())
            return;
        CSSPrimitiveValue* primitiveValue = static_cast<CSSPrimitiveValue*>(value);
        if ((identBehavior == MapNoneToNull && primitiveValue->getIdent() == CSSValueNone)
            || (identBehavior == MapAutoToNull && primitiveValue->getIdent() == CSSValueAuto))
            setValue(selector->style(), nullAtom);
        else
            setValue(selector->style(), primitiveValue->getStringValue());
    }
    static PropertyHandler createHandler()
    {
        PropertyHandler handler = ApplyPropertyDefaultBase<const AtomicString&, getterFunction, const AtomicString&, setterFunction, const AtomicString&, initialFunction>::createHandler();
        return PropertyHandler(handler.inheritFunction(), handler.initialFunction(), &applyValue);
    }
};

template <LengthSize (RenderStyle::*getterFunction)() const, void (RenderStyle::*setterFunction)(LengthSize), LengthSize (*initialFunction)()>
class ApplyPropertyBorderRadius {
public:
    static void setValue(RenderStyle* style, LengthSize value) { (style->*setterFunction)(value); }
    static void applyValue(CSSStyleSelector* selector, CSSValue* value)
    {
        if (!value->isPrimitiveValue())
            return;

        CSSPrimitiveValue* primitiveValue = static_cast<CSSPrimitiveValue*>(value);
        Pair* pair = primitiveValue->getPairValue();
        if (!pair || !pair->first() || !pair->second())
            return;

        Length radiusWidth;
        Length radiusHeight;
        if (pair->first()->primitiveType() == CSSPrimitiveValue::CSS_PERCENTAGE)
            radiusWidth = Length(pair->first()->getDoubleValue(), Percent);
        else
            radiusWidth = Length(max(intMinForLength, min(intMaxForLength, pair->first()->computeLength<int>(selector->style(), selector->rootElementStyle(), selector->style()->effectiveZoom()))), Fixed);
        if (pair->second()->primitiveType() == CSSPrimitiveValue::CSS_PERCENTAGE)
            radiusHeight = Length(pair->second()->getDoubleValue(), Percent);
        else
            radiusHeight = Length(max(intMinForLength, min(intMaxForLength, pair->second()->computeLength<int>(selector->style(), selector->rootElementStyle(), selector->style()->effectiveZoom()))), Fixed);
        int width = radiusWidth.value();
        int height = radiusHeight.value();
        if (width < 0 || height < 0)
            return;
        if (!width)
            radiusHeight = radiusWidth; // Null out the other value.
        else if (!height)
            radiusWidth = radiusHeight; // Null out the other value.

        LengthSize size(radiusWidth, radiusHeight);
        setValue(selector->style(), size);
    }
    static PropertyHandler createHandler()
    {
        PropertyHandler handler = ApplyPropertyDefaultBase<LengthSize, getterFunction, LengthSize, setterFunction, LengthSize, initialFunction>::createHandler();
        return PropertyHandler(handler.inheritFunction(), handler.initialFunction(), &applyValue);
    }
};

template <typename T,
          CSSPropertyID propertyId,
          EFillLayerType fillLayerType,
          FillLayer* (RenderStyle::*accessLayersFunction)(),
          const FillLayer* (RenderStyle::*layersFunction)() const,
          bool (FillLayer::*testFunction)() const,
          T (FillLayer::*getFunction)() const,
          void (FillLayer::*setFunction)(T),
          void (FillLayer::*clearFunction)(),
          T (*initialFunction)(EFillLayerType),
          void (CSSStyleSelector::*mapFillFunction)(CSSPropertyID, FillLayer*, CSSValue*)>
class ApplyPropertyFillLayer {
public:
    static void applyInheritValue(CSSStyleSelector* selector)
    {
        FillLayer* currChild = (selector->style()->*accessLayersFunction)();
        FillLayer* prevChild = 0;
        const FillLayer* currParent = (selector->parentStyle()->*layersFunction)();
        while (currParent && (currParent->*testFunction)()) {
            if (!currChild) {
                /* Need to make a new layer.*/
                currChild = new FillLayer(fillLayerType);
                prevChild->setNext(currChild);
            }
            (currChild->*setFunction)((currParent->*getFunction)());
            prevChild = currChild;
            currChild = prevChild->next();
            currParent = currParent->next();
        }

        while (currChild) {
            /* Reset any remaining layers to not have the property set. */
            (currChild->*clearFunction)();
            currChild = currChild->next();
        }
    }

    static void applyInitialValue(CSSStyleSelector* selector)
    {
        FillLayer* currChild = (selector->style()->*accessLayersFunction)();
        (currChild->*setFunction)((*initialFunction)(fillLayerType));
        for (currChild = currChild->next(); currChild; currChild = currChild->next())
            (currChild->*clearFunction)();
    }

    static void applyValue(CSSStyleSelector* selector, CSSValue* value)
    {
        FillLayer* currChild = (selector->style()->*accessLayersFunction)();
        FillLayer* prevChild = 0;
        if (value->isValueList()) {
            /* Walk each value and put it into a layer, creating new layers as needed. */
            CSSValueList* valueList = static_cast<CSSValueList*>(value);
            for (unsigned int i = 0; i < valueList->length(); i++) {
                if (!currChild) {
                    /* Need to make a new layer to hold this value */
                    currChild = new FillLayer(fillLayerType);
                    prevChild->setNext(currChild);
                }
                (selector->*mapFillFunction)(propertyId, currChild, valueList->itemWithoutBoundsCheck(i));
                prevChild = currChild;
                currChild = currChild->next();
            }
        } else {
            (selector->*mapFillFunction)(propertyId, currChild, value);
            currChild = currChild->next();
        }
        while (currChild) {
            /* Reset all remaining layers to not have the property set. */
            (currChild->*clearFunction)();
            currChild = currChild->next();
        }
    }

    static PropertyHandler createHandler() { return PropertyHandler(&applyInheritValue, &applyInitialValue, &applyValue); }
};

enum ComputeLengthNormal {NormalDisabled = 0, NormalEnabled};
enum ComputeLengthThickness {ThicknessDisabled = 0, ThicknessEnabled};
enum ComputeLengthSVGZoom {SVGZoomDisabled = 0, SVGZoomEnabled};
template <typename T,
          T (RenderStyle::*getterFunction)() const,
          void (RenderStyle::*setterFunction)(T),
          T (*initialFunction)(),
          ComputeLengthNormal normalEnabled = NormalDisabled,
          ComputeLengthThickness thicknessEnabled = ThicknessDisabled,
          ComputeLengthSVGZoom svgZoomEnabled = SVGZoomDisabled>
class ApplyPropertyComputeLength {
public:
    static void setValue(RenderStyle* style, T value) { (style->*setterFunction)(value); }
    static void applyValue(CSSStyleSelector* selector, CSSValue* value)
    {
        // note: CSSPropertyLetter/WordSpacing right now sets to zero if it's not a primitive value for some reason...
        if (!value->isPrimitiveValue())
            return;

        CSSPrimitiveValue* primitiveValue = static_cast<CSSPrimitiveValue*>(value);

        int ident = primitiveValue->getIdent();
        T length;
        if (normalEnabled && ident == CSSValueNormal) {
            length = 0;
        } else if (thicknessEnabled && ident == CSSValueThin) {
            length = 1;
        } else if (thicknessEnabled && ident == CSSValueMedium) {
            length = 3;
        } else if (thicknessEnabled && ident == CSSValueThick) {
            length = 5;
        } else if (ident == CSSValueInvalid) {
            float zoom = (svgZoomEnabled && selector->useSVGZoomRules()) ? 1.0f : selector->style()->effectiveZoom();
            length = primitiveValue->computeLength<T>(selector->style(), selector->rootElementStyle(), zoom);
        } else {
            ASSERT_NOT_REACHED();
            length = 0;
        }

        setValue(selector->style(), length);
    }
    static PropertyHandler createHandler()
    {
        PropertyHandler handler = ApplyPropertyDefaultBase<T, getterFunction, T, setterFunction, T, initialFunction>::createHandler();
        return PropertyHandler(handler.inheritFunction(), handler.initialFunction(), &applyValue);
    }
};

template <typename T, T (FontDescription::*getterFunction)() const, void (FontDescription::*setterFunction)(T), T initialValue>
class ApplyPropertyFont {
public:
    static void applyInheritValue(CSSStyleSelector* selector)
    {
        FontDescription fontDescription = selector->fontDescription();
        (fontDescription.*setterFunction)((selector->parentFontDescription().*getterFunction)());
        selector->setFontDescription(fontDescription);
    }

    static void applyInitialValue(CSSStyleSelector* selector)
    {
        FontDescription fontDescription = selector->fontDescription();
        (fontDescription.*setterFunction)(initialValue);
        selector->setFontDescription(fontDescription);
    }

    static void applyValue(CSSStyleSelector* selector, CSSValue* value)
    {
        if (!value->isPrimitiveValue())
            return;
        CSSPrimitiveValue* primitiveValue = static_cast<CSSPrimitiveValue*>(value);
        FontDescription fontDescription = selector->fontDescription();
        (fontDescription.*setterFunction)(*primitiveValue);
        selector->setFontDescription(fontDescription);
    }

    static PropertyHandler createHandler() { return PropertyHandler(&applyInheritValue, &applyInitialValue, &applyValue); }
};

class ApplyPropertyFontWeight {
public:
    static void applyValue(CSSStyleSelector* selector, CSSValue* value)
    {
        if (!value->isPrimitiveValue())
            return;
        CSSPrimitiveValue* primitiveValue = static_cast<CSSPrimitiveValue*>(value);
        FontDescription fontDescription = selector->fontDescription();
        switch (primitiveValue->getIdent()) {
        case CSSValueInvalid:
            ASSERT_NOT_REACHED();
            break;
        case CSSValueBolder:
            fontDescription.setWeight(fontDescription.bolderWeight());
            break;
        case CSSValueLighter:
            fontDescription.setWeight(fontDescription.lighterWeight());
            break;
        default:
            fontDescription.setWeight(*primitiveValue);
        }
        selector->setFontDescription(fontDescription);
    }
    static PropertyHandler createHandler()
    {
        PropertyHandler handler = ApplyPropertyFont<FontWeight, &FontDescription::weight, &FontDescription::setWeight, FontWeightNormal>::createHandler();
        return PropertyHandler(handler.inheritFunction(), handler.initialFunction(), &applyValue);
    }
};

enum CounterBehavior {Increment = 0, Reset};
template <CounterBehavior counterBehavior>
class ApplyPropertyCounter {
public:
    static void emptyFunction(CSSStyleSelector*) { }
    static void applyValue(CSSStyleSelector* selector, CSSValue* value)
    {
        if (!value->isValueList())
            return;

        CSSValueList* list = static_cast<CSSValueList*>(value);

        CounterDirectiveMap& map = selector->style()->accessCounterDirectives();
        typedef CounterDirectiveMap::iterator Iterator;

        Iterator end = map.end();
        for (Iterator it = map.begin(); it != end; ++it)
            if (counterBehavior)
                it->second.m_reset = false;
            else
                it->second.m_increment = false;

        int length = list ? list->length() : 0;
        for (int i = 0; i < length; ++i) {
            CSSValue* currValue = list->itemWithoutBoundsCheck(i);
            if (!currValue->isPrimitiveValue())
                continue;

            Pair* pair = static_cast<CSSPrimitiveValue*>(currValue)->getPairValue();
            if (!pair || !pair->first() || !pair->second())
                continue;

            AtomicString identifier = static_cast<CSSPrimitiveValue*>(pair->first())->getStringValue();
            // FIXME: What about overflow?
            int value = static_cast<CSSPrimitiveValue*>(pair->second())->getIntValue();
            CounterDirectives& directives = map.add(identifier.impl(), CounterDirectives()).first->second;
            if (counterBehavior) {
                directives.m_reset = true;
                directives.m_resetValue = value;
            } else {
                if (directives.m_increment)
                    directives.m_incrementValue += value;
                else {
                    directives.m_increment = true;
                    directives.m_incrementValue = value;
                }
            }
        }
    }
    static PropertyHandler createHandler() { return PropertyHandler(&emptyFunction, &emptyFunction, &applyValue); }
};


class ApplyPropertyCursor {
public:
    static void applyInheritValue(CSSStyleSelector* selector)
    {
        selector->style()->setCursor(selector->parentStyle()->cursor());
        selector->style()->setCursorList(selector->parentStyle()->cursors());
    }

    static void applyInitialValue(CSSStyleSelector* selector)
    {
        selector->style()->clearCursorList();
        selector->style()->setCursor(RenderStyle::initialCursor());
    }

    static void applyValue(CSSStyleSelector* selector, CSSValue* value)
    {
        selector->style()->clearCursorList();
        if (value->isValueList()) {
            CSSValueList* list = static_cast<CSSValueList*>(value);
            int len = list->length();
            selector->style()->setCursor(CURSOR_AUTO);
            for (int i = 0; i < len; i++) {
                CSSValue* item = list->itemWithoutBoundsCheck(i);
                if (!item->isPrimitiveValue())
                    continue;
                CSSPrimitiveValue* primitiveValue = static_cast<CSSPrimitiveValue*>(item);
                int type = primitiveValue->primitiveType();
                if (type == CSSPrimitiveValue::CSS_URI) {
                    if (primitiveValue->isCursorImageValue()) {
                        CSSCursorImageValue* image = static_cast<CSSCursorImageValue*>(primitiveValue);
                        if (image->updateIfSVGCursorIsUsed(selector->element())) // Elements with SVG cursors are not allowed to share style.
                            selector->style()->setUnique();
                        selector->style()->addCursor(selector->cachedOrPendingFromValue(CSSPropertyCursor, image), image->hotSpot());
                    }
                } else if (type == CSSPrimitiveValue::CSS_IDENT)
                    selector->style()->setCursor(*primitiveValue);
            }
        } else if (value->isPrimitiveValue()) {
            CSSPrimitiveValue* primitiveValue = static_cast<CSSPrimitiveValue*>(value);
            int type = primitiveValue->primitiveType();
            if (type == CSSPrimitiveValue::CSS_IDENT && selector->style()->cursor() != ECursor(*primitiveValue))
                selector->style()->setCursor(*primitiveValue);
        }
    }

    static PropertyHandler createHandler() { return PropertyHandler(&applyInheritValue, &applyInitialValue, &applyValue); }
};

class ApplyPropertyTextEmphasisStyle {
public:
    static void applyInheritValue(CSSStyleSelector* selector)
    {
        selector->style()->setTextEmphasisFill(selector->parentStyle()->textEmphasisFill());
        selector->style()->setTextEmphasisMark(selector->parentStyle()->textEmphasisMark());
        selector->style()->setTextEmphasisCustomMark(selector->parentStyle()->textEmphasisCustomMark());
    }

    static void applyInitialValue(CSSStyleSelector* selector)
    {
        selector->style()->setTextEmphasisFill(RenderStyle::initialTextEmphasisFill());
        selector->style()->setTextEmphasisMark(RenderStyle::initialTextEmphasisMark());
        selector->style()->setTextEmphasisCustomMark(RenderStyle::initialTextEmphasisCustomMark());
    }

    static void applyValue(CSSStyleSelector* selector, CSSValue* value)
    {
        if (value->isValueList()) {
            CSSValueList* list = static_cast<CSSValueList*>(value);
            ASSERT(list->length() == 2);
            if (list->length() != 2)
                return;
            for (unsigned i = 0; i < 2; ++i) {
                CSSValue* item = list->itemWithoutBoundsCheck(i);
                if (!item->isPrimitiveValue())
                    continue;

                CSSPrimitiveValue* value = static_cast<CSSPrimitiveValue*>(item);
                if (value->getIdent() == CSSValueFilled || value->getIdent() == CSSValueOpen)
                    selector->style()->setTextEmphasisFill(*value);
                else
                    selector->style()->setTextEmphasisMark(*value);
            }
            selector->style()->setTextEmphasisCustomMark(nullAtom);
            return;
        }

        if (!value->isPrimitiveValue())
            return;
        CSSPrimitiveValue* primitiveValue = static_cast<CSSPrimitiveValue*>(value);

        if (primitiveValue->primitiveType() == CSSPrimitiveValue::CSS_STRING) {
            selector->style()->setTextEmphasisFill(TextEmphasisFillFilled);
            selector->style()->setTextEmphasisMark(TextEmphasisMarkCustom);
            selector->style()->setTextEmphasisCustomMark(primitiveValue->getStringValue());
            return;
        }

        selector->style()->setTextEmphasisCustomMark(nullAtom);

        if (primitiveValue->getIdent() == CSSValueFilled || primitiveValue->getIdent() == CSSValueOpen) {
            selector->style()->setTextEmphasisFill(*primitiveValue);
            selector->style()->setTextEmphasisMark(TextEmphasisMarkAuto);
        } else {
            selector->style()->setTextEmphasisFill(TextEmphasisFillFilled);
            selector->style()->setTextEmphasisMark(*primitiveValue);
        }
    }

    static PropertyHandler createHandler() { return PropertyHandler(&applyInheritValue, &applyInitialValue, &applyValue); }
};

template <typename T,
          T (Animation::*getterFunction)() const,
          void (Animation::*setterFunction)(T),
          bool (Animation::*testFunction)() const,
          void (Animation::*clearFunction)(),
          T (*initialFunction)(),
          void (CSSStyleSelector::*mapFunction)(Animation*, CSSValue*),
          AnimationList* (RenderStyle::*animationGetterFunction)(),
          const AnimationList* (RenderStyle::*immutableAnimationGetterFunction)() const>
class ApplyPropertyAnimation {
public:
    static void setValue(Animation* animation, T value) { (animation->*setterFunction)(value); }
    static T value(const Animation* animation) { return (animation->*getterFunction)(); }
    static bool test(const Animation* animation) { return (animation->*testFunction)(); }
    static void clear(Animation* animation) { (animation->*clearFunction)(); }
    static T initial() { return (*initialFunction)(); }
    static void map(CSSStyleSelector* selector, Animation* animation, CSSValue* value) { (selector->*mapFunction)(animation, value); }
    static AnimationList* accessAnimations(RenderStyle* style) { return (style->*animationGetterFunction)(); }
    static const AnimationList* animations(RenderStyle* style) { return (style->*immutableAnimationGetterFunction)(); }

    static void applyInheritValue(CSSStyleSelector* selector)
    {
        AnimationList* list = accessAnimations(selector->style());
        const AnimationList* parentList = animations(selector->parentStyle());
        size_t i = 0, parentSize = parentList ? parentList->size() : 0;
        for ( ; i < parentSize && test(parentList->animation(i)); ++i) {
            if (list->size() <= i)
                list->append(Animation::create());
            setValue(list->animation(i), value(parentList->animation(i)));
        }

        /* Reset any remaining animations to not have the property set. */
        for ( ; i < list->size(); ++i)
            clear(list->animation(i));
    }

    static void applyInitialValue(CSSStyleSelector* selector)
    {
        AnimationList* list = accessAnimations(selector->style());
        if (list->isEmpty())
            list->append(Animation::create());
        setValue(list->animation(0), initial());
        for (size_t i = 1; i < list->size(); ++i)
            clear(list->animation(i));
    }

    static void applyValue(CSSStyleSelector* selector, CSSValue* value)
    {
        AnimationList* list = accessAnimations(selector->style());
        size_t childIndex = 0;
        if (value->isValueList()) {
            /* Walk each value and put it into an animation, creating new animations as needed. */
            for (CSSValueListIterator i = value; i.hasMore(); i.advance()) {
                if (childIndex <= list->size())
                    list->append(Animation::create());
                map(selector, list->animation(childIndex), i.value());
                ++childIndex;
            }
        } else {
            if (list->isEmpty())
                list->append(Animation::create());
            map(selector, list->animation(childIndex), value);
            childIndex = 1;
        }
        for ( ; childIndex < list->size(); ++childIndex) {
            /* Reset all remaining animations to not have the property set. */
            clear(list->animation(childIndex));
        }
    }

    static PropertyHandler createHandler() { return PropertyHandler(&applyInheritValue, &applyInitialValue, &applyValue); }
};

class ApplyPropertyOutlineStyle {
public:
    static void applyInheritValue(CSSStyleSelector* selector)
    {
        ApplyPropertyDefaultBase<OutlineIsAuto, &RenderStyle::outlineStyleIsAuto, OutlineIsAuto, &RenderStyle::setOutlineStyleIsAuto, OutlineIsAuto, &RenderStyle::initialOutlineStyleIsAuto>::applyInheritValue(selector);
        ApplyPropertyDefaultBase<EBorderStyle, &RenderStyle::outlineStyle, EBorderStyle, &RenderStyle::setOutlineStyle, EBorderStyle, &RenderStyle::initialBorderStyle>::applyInheritValue(selector);
    }

    static void applyInitialValue(CSSStyleSelector* selector)
    {
        ApplyPropertyDefaultBase<OutlineIsAuto, &RenderStyle::outlineStyleIsAuto, OutlineIsAuto, &RenderStyle::setOutlineStyleIsAuto, OutlineIsAuto, &RenderStyle::initialOutlineStyleIsAuto>::applyInitialValue(selector);
        ApplyPropertyDefaultBase<EBorderStyle, &RenderStyle::outlineStyle, EBorderStyle, &RenderStyle::setOutlineStyle, EBorderStyle, &RenderStyle::initialBorderStyle>::applyInitialValue(selector);
    }

    static void applyValue(CSSStyleSelector* selector, CSSValue* value)
    {
        ApplyPropertyDefault<OutlineIsAuto, &RenderStyle::outlineStyleIsAuto, OutlineIsAuto, &RenderStyle::setOutlineStyleIsAuto, OutlineIsAuto, &RenderStyle::initialOutlineStyleIsAuto>::applyValue(selector, value);
        ApplyPropertyDefault<EBorderStyle, &RenderStyle::outlineStyle, EBorderStyle, &RenderStyle::setOutlineStyle, EBorderStyle, &RenderStyle::initialBorderStyle>::applyValue(selector, value);
    }

    static PropertyHandler createHandler() { return PropertyHandler(&applyInheritValue, &applyInitialValue, &applyValue); }
};

const CSSStyleApplyProperty& CSSStyleApplyProperty::sharedCSSStyleApplyProperty()
{
    DEFINE_STATIC_LOCAL(CSSStyleApplyProperty, cssStyleApplyPropertyInstance, ());
    return cssStyleApplyPropertyInstance;
}

CSSStyleApplyProperty::CSSStyleApplyProperty()
{
    for (int i = 0; i < numCSSProperties; ++i)
        m_propertyMap[i] = PropertyHandler();

    setPropertyHandler(CSSPropertyColor, ApplyPropertyColor<InheritFromParent, &RenderStyle::color, &RenderStyle::setColor, &RenderStyle::setVisitedLinkColor, &RenderStyle::invalidColor, RenderStyle::initialColor>::createHandler());
    setPropertyHandler(CSSPropertyDirection, ApplyPropertyDirection<&RenderStyle::direction, &RenderStyle::setDirection, RenderStyle::initialDirection>::createHandler());

    setPropertyHandler(CSSPropertyBackgroundAttachment, ApplyPropertyFillLayer<EFillAttachment, CSSPropertyBackgroundAttachment, BackgroundFillLayer, &RenderStyle::accessBackgroundLayers, &RenderStyle::backgroundLayers,
                       &FillLayer::isAttachmentSet, &FillLayer::attachment, &FillLayer::setAttachment, &FillLayer::clearAttachment, &FillLayer::initialFillAttachment, &CSSStyleSelector::mapFillAttachment>::createHandler());
    setPropertyHandler(CSSPropertyBackgroundClip, ApplyPropertyFillLayer<EFillBox, CSSPropertyBackgroundClip, BackgroundFillLayer, &RenderStyle::accessBackgroundLayers, &RenderStyle::backgroundLayers,
                       &FillLayer::isClipSet, &FillLayer::clip, &FillLayer::setClip, &FillLayer::clearClip, &FillLayer::initialFillClip, &CSSStyleSelector::mapFillClip>::createHandler());
    setPropertyHandler(CSSPropertyWebkitBackgroundClip, CSSPropertyBackgroundClip);
    setPropertyHandler(CSSPropertyWebkitBackgroundComposite, ApplyPropertyFillLayer<CompositeOperator, CSSPropertyWebkitBackgroundComposite, BackgroundFillLayer, &RenderStyle::accessBackgroundLayers, &RenderStyle::backgroundLayers,
                       &FillLayer::isCompositeSet, &FillLayer::composite, &FillLayer::setComposite, &FillLayer::clearComposite, &FillLayer::initialFillComposite, &CSSStyleSelector::mapFillComposite>::createHandler());

    setPropertyHandler(CSSPropertyBackgroundImage, ApplyPropertyFillLayer<StyleImage*, CSSPropertyBackgroundImage, BackgroundFillLayer, &RenderStyle::accessBackgroundLayers, &RenderStyle::backgroundLayers,
                       &FillLayer::isImageSet, &FillLayer::image, &FillLayer::setImage, &FillLayer::clearImage, &FillLayer::initialFillImage, &CSSStyleSelector::mapFillImage>::createHandler());

    setPropertyHandler(CSSPropertyBackgroundOrigin, ApplyPropertyFillLayer<EFillBox, CSSPropertyBackgroundOrigin, BackgroundFillLayer, &RenderStyle::accessBackgroundLayers, &RenderStyle::backgroundLayers,
                       &FillLayer::isOriginSet, &FillLayer::origin, &FillLayer::setOrigin, &FillLayer::clearOrigin, &FillLayer::initialFillOrigin, &CSSStyleSelector::mapFillOrigin>::createHandler());
    setPropertyHandler(CSSPropertyWebkitBackgroundOrigin, CSSPropertyBackgroundOrigin);

    setPropertyHandler(CSSPropertyBackgroundPositionX, ApplyPropertyFillLayer<Length, CSSPropertyBackgroundPositionX, BackgroundFillLayer, &RenderStyle::accessBackgroundLayers, &RenderStyle::backgroundLayers,
                       &FillLayer::isXPositionSet, &FillLayer::xPosition, &FillLayer::setXPosition, &FillLayer::clearXPosition, &FillLayer::initialFillXPosition, &CSSStyleSelector::mapFillXPosition>::createHandler());
    setPropertyHandler(CSSPropertyBackgroundPositionY, ApplyPropertyFillLayer<Length, CSSPropertyBackgroundPositionY, BackgroundFillLayer, &RenderStyle::accessBackgroundLayers, &RenderStyle::backgroundLayers,
                       &FillLayer::isYPositionSet, &FillLayer::yPosition, &FillLayer::setYPosition, &FillLayer::clearYPosition, &FillLayer::initialFillYPosition, &CSSStyleSelector::mapFillYPosition>::createHandler());
    setPropertyHandler(CSSPropertyBackgroundPosition, ApplyPropertyExpanding<SuppressValue, CSSPropertyBackgroundPositionX, CSSPropertyBackgroundPositionY>::createHandler());

    setPropertyHandler(CSSPropertyBackgroundRepeatX, ApplyPropertyFillLayer<EFillRepeat, CSSPropertyBackgroundRepeatX, BackgroundFillLayer, &RenderStyle::accessBackgroundLayers, &RenderStyle::backgroundLayers,
                       &FillLayer::isRepeatXSet, &FillLayer::repeatX, &FillLayer::setRepeatX, &FillLayer::clearRepeatX, &FillLayer::initialFillRepeatX, &CSSStyleSelector::mapFillRepeatX>::createHandler());
    setPropertyHandler(CSSPropertyBackgroundRepeatY, ApplyPropertyFillLayer<EFillRepeat, CSSPropertyBackgroundRepeatY, BackgroundFillLayer, &RenderStyle::accessBackgroundLayers, &RenderStyle::backgroundLayers,
                       &FillLayer::isRepeatYSet, &FillLayer::repeatY, &FillLayer::setRepeatY, &FillLayer::clearRepeatY, &FillLayer::initialFillRepeatY, &CSSStyleSelector::mapFillRepeatY>::createHandler());
    setPropertyHandler(CSSPropertyBackgroundRepeat, ApplyPropertyExpanding<SuppressValue, CSSPropertyBackgroundRepeatX, CSSPropertyBackgroundRepeatY>::createHandler());

    setPropertyHandler(CSSPropertyBackgroundSize, ApplyPropertyFillLayer<FillSize, CSSPropertyBackgroundSize, BackgroundFillLayer, &RenderStyle::accessBackgroundLayers, &RenderStyle::backgroundLayers,
                       &FillLayer::isSizeSet, &FillLayer::size, &FillLayer::setSize, &FillLayer::clearSize, &FillLayer::initialFillSize, &CSSStyleSelector::mapFillSize>::createHandler());
    setPropertyHandler(CSSPropertyWebkitBackgroundSize, CSSPropertyBackgroundSize);

    setPropertyHandler(CSSPropertyWebkitMaskAttachment, ApplyPropertyFillLayer<EFillAttachment, CSSPropertyWebkitMaskAttachment, MaskFillLayer, &RenderStyle::accessMaskLayers, &RenderStyle::maskLayers,
                       &FillLayer::isAttachmentSet, &FillLayer::attachment, &FillLayer::setAttachment, &FillLayer::clearAttachment, &FillLayer::initialFillAttachment, &CSSStyleSelector::mapFillAttachment>::createHandler());
    setPropertyHandler(CSSPropertyWebkitMaskClip, ApplyPropertyFillLayer<EFillBox, CSSPropertyWebkitMaskClip, MaskFillLayer, &RenderStyle::accessMaskLayers, &RenderStyle::maskLayers,
                       &FillLayer::isClipSet, &FillLayer::clip, &FillLayer::setClip, &FillLayer::clearClip, &FillLayer::initialFillClip, &CSSStyleSelector::mapFillClip>::createHandler());
    setPropertyHandler(CSSPropertyWebkitMaskComposite, ApplyPropertyFillLayer<CompositeOperator, CSSPropertyWebkitMaskComposite, MaskFillLayer, &RenderStyle::accessMaskLayers, &RenderStyle::maskLayers,
                       &FillLayer::isCompositeSet, &FillLayer::composite, &FillLayer::setComposite, &FillLayer::clearComposite, &FillLayer::initialFillComposite, &CSSStyleSelector::mapFillComposite>::createHandler());

    setPropertyHandler(CSSPropertyWebkitMaskImage, ApplyPropertyFillLayer<StyleImage*, CSSPropertyWebkitMaskImage, MaskFillLayer, &RenderStyle::accessMaskLayers, &RenderStyle::maskLayers,
                       &FillLayer::isImageSet, &FillLayer::image, &FillLayer::setImage, &FillLayer::clearImage, &FillLayer::initialFillImage, &CSSStyleSelector::mapFillImage>::createHandler());

    setPropertyHandler(CSSPropertyWebkitMaskOrigin, ApplyPropertyFillLayer<EFillBox, CSSPropertyWebkitMaskOrigin, MaskFillLayer, &RenderStyle::accessMaskLayers, &RenderStyle::maskLayers,
                       &FillLayer::isOriginSet, &FillLayer::origin, &FillLayer::setOrigin, &FillLayer::clearOrigin, &FillLayer::initialFillOrigin, &CSSStyleSelector::mapFillOrigin>::createHandler());
    setPropertyHandler(CSSPropertyWebkitMaskSize, ApplyPropertyFillLayer<FillSize, CSSPropertyWebkitMaskSize, MaskFillLayer, &RenderStyle::accessMaskLayers, &RenderStyle::maskLayers,
                       &FillLayer::isSizeSet, &FillLayer::size, &FillLayer::setSize, &FillLayer::clearSize, &FillLayer::initialFillSize, &CSSStyleSelector::mapFillSize>::createHandler());

    setPropertyHandler(CSSPropertyWebkitMaskPositionX, ApplyPropertyFillLayer<Length, CSSPropertyWebkitMaskPositionX, MaskFillLayer, &RenderStyle::accessMaskLayers, &RenderStyle::maskLayers,
                       &FillLayer::isXPositionSet, &FillLayer::xPosition, &FillLayer::setXPosition, &FillLayer::clearXPosition, &FillLayer::initialFillXPosition, &CSSStyleSelector::mapFillXPosition>::createHandler());
    setPropertyHandler(CSSPropertyWebkitMaskPositionY, ApplyPropertyFillLayer<Length, CSSPropertyWebkitMaskPositionY, MaskFillLayer, &RenderStyle::accessMaskLayers, &RenderStyle::maskLayers,
                       &FillLayer::isYPositionSet, &FillLayer::yPosition, &FillLayer::setYPosition, &FillLayer::clearYPosition, &FillLayer::initialFillYPosition, &CSSStyleSelector::mapFillYPosition>::createHandler());
    setPropertyHandler(CSSPropertyWebkitMaskPosition, ApplyPropertyExpanding<SuppressValue, CSSPropertyWebkitMaskPositionX, CSSPropertyWebkitMaskPositionY>::createHandler());

    setPropertyHandler(CSSPropertyWebkitMaskRepeatX, ApplyPropertyFillLayer<EFillRepeat, CSSPropertyWebkitMaskRepeatX, MaskFillLayer, &RenderStyle::accessMaskLayers, &RenderStyle::maskLayers,
                       &FillLayer::isRepeatXSet, &FillLayer::repeatX, &FillLayer::setRepeatX, &FillLayer::clearRepeatX, &FillLayer::initialFillRepeatX, &CSSStyleSelector::mapFillRepeatX>::createHandler());
    setPropertyHandler(CSSPropertyWebkitMaskRepeatY, ApplyPropertyFillLayer<EFillRepeat, CSSPropertyWebkitMaskRepeatY, MaskFillLayer, &RenderStyle::accessMaskLayers, &RenderStyle::maskLayers,
                       &FillLayer::isRepeatYSet, &FillLayer::repeatY, &FillLayer::setRepeatY, &FillLayer::clearRepeatY, &FillLayer::initialFillRepeatY, &CSSStyleSelector::mapFillRepeatY>::createHandler());
    setPropertyHandler(CSSPropertyWebkitMaskRepeat, ApplyPropertyExpanding<SuppressValue, CSSPropertyBackgroundRepeatX, CSSPropertyBackgroundRepeatY>::createHandler());

    setPropertyHandler(CSSPropertyBackgroundColor, ApplyPropertyColor<NoInheritFromParent, &RenderStyle::backgroundColor, &RenderStyle::setBackgroundColor, &RenderStyle::setVisitedLinkBackgroundColor, &RenderStyle::invalidColor>::createHandler());
    setPropertyHandler(CSSPropertyBorderBottomColor, ApplyPropertyColor<NoInheritFromParent, &RenderStyle::borderBottomColor, &RenderStyle::setBorderBottomColor, &RenderStyle::setVisitedLinkBorderBottomColor, &RenderStyle::color>::createHandler());
    setPropertyHandler(CSSPropertyBorderLeftColor, ApplyPropertyColor<NoInheritFromParent, &RenderStyle::borderLeftColor, &RenderStyle::setBorderLeftColor, &RenderStyle::setVisitedLinkBorderLeftColor, &RenderStyle::color>::createHandler());
    setPropertyHandler(CSSPropertyBorderRightColor, ApplyPropertyColor<NoInheritFromParent, &RenderStyle::borderRightColor, &RenderStyle::setBorderRightColor, &RenderStyle::setVisitedLinkBorderRightColor, &RenderStyle::color>::createHandler());
    setPropertyHandler(CSSPropertyBorderTopColor, ApplyPropertyColor<NoInheritFromParent, &RenderStyle::borderTopColor, &RenderStyle::setBorderTopColor, &RenderStyle::setVisitedLinkBorderTopColor, &RenderStyle::color>::createHandler());

    setPropertyHandler(CSSPropertyBorderTopStyle, ApplyPropertyDefault<EBorderStyle, &RenderStyle::borderTopStyle, EBorderStyle, &RenderStyle::setBorderTopStyle, EBorderStyle, &RenderStyle::initialBorderStyle>::createHandler());
    setPropertyHandler(CSSPropertyBorderRightStyle, ApplyPropertyDefault<EBorderStyle, &RenderStyle::borderRightStyle, EBorderStyle, &RenderStyle::setBorderRightStyle, EBorderStyle, &RenderStyle::initialBorderStyle>::createHandler());
    setPropertyHandler(CSSPropertyBorderBottomStyle, ApplyPropertyDefault<EBorderStyle, &RenderStyle::borderBottomStyle, EBorderStyle, &RenderStyle::setBorderBottomStyle, EBorderStyle, &RenderStyle::initialBorderStyle>::createHandler());
    setPropertyHandler(CSSPropertyBorderLeftStyle, ApplyPropertyDefault<EBorderStyle, &RenderStyle::borderLeftStyle, EBorderStyle, &RenderStyle::setBorderLeftStyle, EBorderStyle, &RenderStyle::initialBorderStyle>::createHandler());

    setPropertyHandler(CSSPropertyBorderTopWidth, ApplyPropertyComputeLength<unsigned short, &RenderStyle::borderTopWidth, &RenderStyle::setBorderTopWidth, &RenderStyle::initialBorderWidth, NormalDisabled, ThicknessEnabled>::createHandler());
    setPropertyHandler(CSSPropertyBorderRightWidth, ApplyPropertyComputeLength<unsigned short, &RenderStyle::borderRightWidth, &RenderStyle::setBorderRightWidth, &RenderStyle::initialBorderWidth, NormalDisabled, ThicknessEnabled>::createHandler());
    setPropertyHandler(CSSPropertyBorderBottomWidth, ApplyPropertyComputeLength<unsigned short, &RenderStyle::borderBottomWidth, &RenderStyle::setBorderBottomWidth, &RenderStyle::initialBorderWidth, NormalDisabled, ThicknessEnabled>::createHandler());
    setPropertyHandler(CSSPropertyBorderLeftWidth, ApplyPropertyComputeLength<unsigned short, &RenderStyle::borderLeftWidth, &RenderStyle::setBorderLeftWidth, &RenderStyle::initialBorderWidth, NormalDisabled, ThicknessEnabled>::createHandler());
    setPropertyHandler(CSSPropertyOutlineWidth, ApplyPropertyComputeLength<unsigned short, &RenderStyle::outlineWidth, &RenderStyle::setOutlineWidth, &RenderStyle::initialBorderWidth, NormalDisabled, ThicknessEnabled>::createHandler());
    setPropertyHandler(CSSPropertyWebkitColumnRuleWidth, ApplyPropertyComputeLength<unsigned short, &RenderStyle::columnRuleWidth, &RenderStyle::setColumnRuleWidth, &RenderStyle::initialBorderWidth, NormalDisabled, ThicknessEnabled>::createHandler());

    setPropertyHandler(CSSPropertyBorderTop, ApplyPropertyExpanding<SuppressValue, CSSPropertyBorderTopColor, CSSPropertyBorderTopStyle, CSSPropertyBorderTopWidth>::createHandler());
    setPropertyHandler(CSSPropertyBorderRight, ApplyPropertyExpanding<SuppressValue, CSSPropertyBorderRightColor, CSSPropertyBorderRightStyle, CSSPropertyBorderRightWidth>::createHandler());
    setPropertyHandler(CSSPropertyBorderBottom, ApplyPropertyExpanding<SuppressValue, CSSPropertyBorderBottomColor, CSSPropertyBorderBottomStyle, CSSPropertyBorderBottomWidth>::createHandler());
    setPropertyHandler(CSSPropertyBorderLeft, ApplyPropertyExpanding<SuppressValue, CSSPropertyBorderLeftColor, CSSPropertyBorderLeftStyle, CSSPropertyBorderLeftWidth>::createHandler());

    setPropertyHandler(CSSPropertyBorderStyle, ApplyPropertyExpanding<SuppressValue, CSSPropertyBorderTopStyle, CSSPropertyBorderRightStyle, CSSPropertyBorderBottomStyle, CSSPropertyBorderLeftStyle>::createHandler());
    setPropertyHandler(CSSPropertyBorderWidth, ApplyPropertyExpanding<SuppressValue, CSSPropertyBorderTopWidth, CSSPropertyBorderRightWidth, CSSPropertyBorderBottomWidth, CSSPropertyBorderLeftWidth>::createHandler());
    setPropertyHandler(CSSPropertyBorderColor, ApplyPropertyExpanding<SuppressValue, CSSPropertyBorderTopColor, CSSPropertyBorderRightColor, CSSPropertyBorderBottomColor, CSSPropertyBorderLeftColor>::createHandler());
    setPropertyHandler(CSSPropertyBorder, ApplyPropertyExpanding<SuppressValue, CSSPropertyBorderStyle, CSSPropertyBorderWidth, CSSPropertyBorderColor>::createHandler());

    setPropertyHandler(CSSPropertyBorderTopLeftRadius, ApplyPropertyBorderRadius<&RenderStyle::borderTopLeftRadius, &RenderStyle::setBorderTopLeftRadius, &RenderStyle::initialBorderRadius>::createHandler());
    setPropertyHandler(CSSPropertyBorderTopRightRadius, ApplyPropertyBorderRadius<&RenderStyle::borderTopRightRadius, &RenderStyle::setBorderTopRightRadius, &RenderStyle::initialBorderRadius>::createHandler());
    setPropertyHandler(CSSPropertyBorderBottomLeftRadius, ApplyPropertyBorderRadius<&RenderStyle::borderBottomLeftRadius, &RenderStyle::setBorderBottomLeftRadius, &RenderStyle::initialBorderRadius>::createHandler());
    setPropertyHandler(CSSPropertyBorderBottomRightRadius, ApplyPropertyBorderRadius<&RenderStyle::borderBottomRightRadius, &RenderStyle::setBorderBottomRightRadius, &RenderStyle::initialBorderRadius>::createHandler());
    setPropertyHandler(CSSPropertyBorderRadius, ApplyPropertyExpanding<ExpandValue, CSSPropertyBorderTopLeftRadius, CSSPropertyBorderTopRightRadius, CSSPropertyBorderBottomLeftRadius, CSSPropertyBorderBottomRightRadius>::createHandler());
    setPropertyHandler(CSSPropertyWebkitBorderRadius, CSSPropertyBorderRadius);

    setPropertyHandler(CSSPropertyWebkitBorderHorizontalSpacing, ApplyPropertyComputeLength<short, &RenderStyle::horizontalBorderSpacing, &RenderStyle::setHorizontalBorderSpacing, &RenderStyle::initialHorizontalBorderSpacing>::createHandler());
    setPropertyHandler(CSSPropertyWebkitBorderVerticalSpacing, ApplyPropertyComputeLength<short, &RenderStyle::verticalBorderSpacing, &RenderStyle::setVerticalBorderSpacing, &RenderStyle::initialVerticalBorderSpacing>::createHandler());
    setPropertyHandler(CSSPropertyBorderSpacing, ApplyPropertyExpanding<SuppressValue, CSSPropertyWebkitBorderHorizontalSpacing, CSSPropertyWebkitBorderVerticalSpacing>::createHandler());

    setPropertyHandler(CSSPropertyLetterSpacing, ApplyPropertyComputeLength<int, &RenderStyle::letterSpacing, &RenderStyle::setLetterSpacing, &RenderStyle::initialLetterWordSpacing, NormalEnabled, ThicknessDisabled, SVGZoomEnabled>::createHandler());
    setPropertyHandler(CSSPropertyWordSpacing, ApplyPropertyComputeLength<int, &RenderStyle::wordSpacing, &RenderStyle::setWordSpacing, &RenderStyle::initialLetterWordSpacing, NormalEnabled, ThicknessDisabled, SVGZoomEnabled>::createHandler());

    setPropertyHandler(CSSPropertyCursor, ApplyPropertyCursor::createHandler());

    setPropertyHandler(CSSPropertyCounterIncrement, ApplyPropertyCounter<Increment>::createHandler());
    setPropertyHandler(CSSPropertyCounterReset, ApplyPropertyCounter<Reset>::createHandler());

#if ENABLE(CSS3_FLEXBOX)
    setPropertyHandler(CSSPropertyWebkitFlexOrder, ApplyPropertyDefault<int, &RenderStyle::flexOrder, int, &RenderStyle::setFlexOrder, int, &RenderStyle::initialFlexOrder>::createHandler());
    setPropertyHandler(CSSPropertyWebkitFlexPack, ApplyPropertyDefault<EFlexPack, &RenderStyle::flexPack, EFlexPack, &RenderStyle::setFlexPack, EFlexPack, &RenderStyle::initialFlexPack>::createHandler());
    setPropertyHandler(CSSPropertyWebkitFlexAlign, ApplyPropertyDefault<EFlexAlign, &RenderStyle::flexAlign, EFlexAlign, &RenderStyle::setFlexAlign, EFlexAlign, &RenderStyle::initialFlexAlign>::createHandler());
    setPropertyHandler(CSSPropertyWebkitFlexFlow, ApplyPropertyDefault<EFlexFlow, &RenderStyle::flexFlow, EFlexFlow, &RenderStyle::setFlexFlow, EFlexFlow, &RenderStyle::initialFlexFlow>::createHandler());
#endif

    setPropertyHandler(CSSPropertyFontStyle, ApplyPropertyFont<FontItalic, &FontDescription::italic, &FontDescription::setItalic, FontItalicOff>::createHandler());
    setPropertyHandler(CSSPropertyFontVariant, ApplyPropertyFont<FontSmallCaps, &FontDescription::smallCaps, &FontDescription::setSmallCaps, FontSmallCapsOff>::createHandler());
    setPropertyHandler(CSSPropertyTextRendering, ApplyPropertyFont<TextRenderingMode, &FontDescription::textRenderingMode, &FontDescription::setTextRenderingMode, AutoTextRendering>::createHandler());
    setPropertyHandler(CSSPropertyWebkitFontSmoothing, ApplyPropertyFont<FontSmoothingMode, &FontDescription::fontSmoothing, &FontDescription::setFontSmoothing, AutoSmoothing>::createHandler());
    setPropertyHandler(CSSPropertyWebkitTextOrientation, ApplyPropertyFont<TextOrientation, &FontDescription::textOrientation, &FontDescription::setTextOrientation, TextOrientationVerticalRight>::createHandler());
    setPropertyHandler(CSSPropertyFontWeight, ApplyPropertyFontWeight::createHandler());

    setPropertyHandler(CSSPropertyOutlineStyle, ApplyPropertyOutlineStyle::createHandler());
    setPropertyHandler(CSSPropertyOutlineColor, ApplyPropertyColor<InheritFromParent, &RenderStyle::outlineColor, &RenderStyle::setOutlineColor, &RenderStyle::setVisitedLinkOutlineColor, &RenderStyle::color>::createHandler());
    setPropertyHandler(CSSPropertyOutlineOffset, ApplyPropertyComputeLength<int, &RenderStyle::outlineOffset, &RenderStyle::setOutlineOffset, &RenderStyle::initialOutlineOffset>::createHandler());

    setPropertyHandler(CSSPropertyOverflowX, ApplyPropertyDefault<EOverflow, &RenderStyle::overflowX, EOverflow, &RenderStyle::setOverflowX, EOverflow, &RenderStyle::initialOverflowX>::createHandler());
    setPropertyHandler(CSSPropertyOverflowY, ApplyPropertyDefault<EOverflow, &RenderStyle::overflowY, EOverflow, &RenderStyle::setOverflowY, EOverflow, &RenderStyle::initialOverflowY>::createHandler());
    setPropertyHandler(CSSPropertyOverflow, ApplyPropertyExpanding<ExpandValue, CSSPropertyOverflowX, CSSPropertyOverflowY>::createHandler());

    setPropertyHandler(CSSPropertyWebkitColumnRuleColor, ApplyPropertyColor<NoInheritFromParent, &RenderStyle::columnRuleColor, &RenderStyle::setColumnRuleColor, &RenderStyle::setVisitedLinkColumnRuleColor, &RenderStyle::color>::createHandler());
    setPropertyHandler(CSSPropertyWebkitTextEmphasisColor, ApplyPropertyColor<NoInheritFromParent, &RenderStyle::textEmphasisColor, &RenderStyle::setTextEmphasisColor, &RenderStyle::setVisitedLinkTextEmphasisColor, &RenderStyle::color>::createHandler());
    setPropertyHandler(CSSPropertyWebkitTextFillColor, ApplyPropertyColor<NoInheritFromParent, &RenderStyle::textFillColor, &RenderStyle::setTextFillColor, &RenderStyle::setVisitedLinkTextFillColor, &RenderStyle::color>::createHandler());
    setPropertyHandler(CSSPropertyWebkitTextStrokeColor, ApplyPropertyColor<NoInheritFromParent, &RenderStyle::textStrokeColor, &RenderStyle::setTextStrokeColor, &RenderStyle::setVisitedLinkTextStrokeColor, &RenderStyle::color>::createHandler());

    setPropertyHandler(CSSPropertyTop, ApplyPropertyLength<&RenderStyle::top, &RenderStyle::setTop, &RenderStyle::initialOffset, AutoEnabled>::createHandler());
    setPropertyHandler(CSSPropertyRight, ApplyPropertyLength<&RenderStyle::right, &RenderStyle::setRight, &RenderStyle::initialOffset, AutoEnabled>::createHandler());
    setPropertyHandler(CSSPropertyBottom, ApplyPropertyLength<&RenderStyle::bottom, &RenderStyle::setBottom, &RenderStyle::initialOffset, AutoEnabled>::createHandler());
    setPropertyHandler(CSSPropertyLeft, ApplyPropertyLength<&RenderStyle::left, &RenderStyle::setLeft, &RenderStyle::initialOffset, AutoEnabled>::createHandler());

    setPropertyHandler(CSSPropertyWidth, ApplyPropertyLength<&RenderStyle::width, &RenderStyle::setWidth, &RenderStyle::initialSize, AutoEnabled, IntrinsicEnabled, MinIntrinsicEnabled, NoneDisabled, UndefinedDisabled, FlexWidth>::createHandler());
    setPropertyHandler(CSSPropertyHeight, ApplyPropertyLength<&RenderStyle::height, &RenderStyle::setHeight, &RenderStyle::initialSize, AutoEnabled, IntrinsicEnabled, MinIntrinsicEnabled, NoneDisabled, UndefinedDisabled, FlexHeight>::createHandler());

    setPropertyHandler(CSSPropertyTextIndent, ApplyPropertyLength<&RenderStyle::textIndent, &RenderStyle::setTextIndent, &RenderStyle::initialTextIndent>::createHandler());

    setPropertyHandler(CSSPropertyListStyleImage, ApplyPropertyStyleImage<&RenderStyle::listStyleImage, &RenderStyle::setListStyleImage, &RenderStyle::initialListStyleImage, CSSPropertyListStyleImage>::createHandler());
    setPropertyHandler(CSSPropertyListStylePosition, ApplyPropertyDefault<EListStylePosition, &RenderStyle::listStylePosition, EListStylePosition, &RenderStyle::setListStylePosition, EListStylePosition, &RenderStyle::initialListStylePosition>::createHandler());
    setPropertyHandler(CSSPropertyListStyleType, ApplyPropertyDefault<EListStyleType, &RenderStyle::listStyleType, EListStyleType, &RenderStyle::setListStyleType, EListStyleType, &RenderStyle::initialListStyleType>::createHandler());
    setPropertyHandler(CSSPropertyListStyle, ApplyPropertyExpanding<SuppressValue, CSSPropertyListStyleType, CSSPropertyListStyleImage, CSSPropertyListStylePosition>::createHandler());

    setPropertyHandler(CSSPropertyMaxHeight, ApplyPropertyLength<&RenderStyle::maxHeight, &RenderStyle::setMaxHeight, &RenderStyle::initialMaxSize, AutoEnabled, IntrinsicEnabled, MinIntrinsicEnabled, NoneEnabled, UndefinedEnabled>::createHandler());
    setPropertyHandler(CSSPropertyMaxWidth, ApplyPropertyLength<&RenderStyle::maxWidth, &RenderStyle::setMaxWidth, &RenderStyle::initialMaxSize, AutoEnabled, IntrinsicEnabled, MinIntrinsicEnabled, NoneEnabled, UndefinedEnabled>::createHandler());
    setPropertyHandler(CSSPropertyMinHeight, ApplyPropertyLength<&RenderStyle::minHeight, &RenderStyle::setMinHeight, &RenderStyle::initialMinSize, AutoEnabled, IntrinsicEnabled, MinIntrinsicEnabled>::createHandler());
    setPropertyHandler(CSSPropertyMinWidth, ApplyPropertyLength<&RenderStyle::minWidth, &RenderStyle::setMinWidth, &RenderStyle::initialMinSize, AutoEnabled, IntrinsicEnabled, MinIntrinsicEnabled>::createHandler());

    setPropertyHandler(CSSPropertyMarginTop, ApplyPropertyLength<&RenderStyle::marginTop, &RenderStyle::setMarginTop, &RenderStyle::initialMargin, AutoEnabled>::createHandler());
    setPropertyHandler(CSSPropertyMarginRight, ApplyPropertyLength<&RenderStyle::marginRight, &RenderStyle::setMarginRight, &RenderStyle::initialMargin, AutoEnabled>::createHandler());
    setPropertyHandler(CSSPropertyMarginBottom, ApplyPropertyLength<&RenderStyle::marginBottom, &RenderStyle::setMarginBottom, &RenderStyle::initialMargin, AutoEnabled>::createHandler());
    setPropertyHandler(CSSPropertyMarginLeft, ApplyPropertyLength<&RenderStyle::marginLeft, &RenderStyle::setMarginLeft, &RenderStyle::initialMargin, AutoEnabled>::createHandler());
    setPropertyHandler(CSSPropertyMargin, ApplyPropertyExpanding<SuppressValue, CSSPropertyMarginTop, CSSPropertyMarginRight, CSSPropertyMarginBottom, CSSPropertyMarginLeft>::createHandler());

    setPropertyHandler(CSSPropertyWebkitMarginBeforeCollapse, ApplyPropertyDefault<EMarginCollapse, &RenderStyle::marginBeforeCollapse, EMarginCollapse, &RenderStyle::setMarginBeforeCollapse, EMarginCollapse, &RenderStyle::initialMarginBeforeCollapse>::createHandler());
    setPropertyHandler(CSSPropertyWebkitMarginAfterCollapse, ApplyPropertyDefault<EMarginCollapse, &RenderStyle::marginAfterCollapse, EMarginCollapse, &RenderStyle::setMarginAfterCollapse, EMarginCollapse, &RenderStyle::initialMarginAfterCollapse>::createHandler());
    setPropertyHandler(CSSPropertyWebkitMarginTopCollapse, CSSPropertyWebkitMarginBeforeCollapse);
    setPropertyHandler(CSSPropertyWebkitMarginBottomCollapse, CSSPropertyWebkitMarginAfterCollapse);
    setPropertyHandler(CSSPropertyWebkitMarginCollapse, ApplyPropertyExpanding<SuppressValue, CSSPropertyWebkitMarginBeforeCollapse, CSSPropertyWebkitMarginAfterCollapse>::createHandler());

    setPropertyHandler(CSSPropertyPaddingTop, ApplyPropertyLength<&RenderStyle::paddingTop, &RenderStyle::setPaddingTop, &RenderStyle::initialPadding>::createHandler());
    setPropertyHandler(CSSPropertyPaddingRight, ApplyPropertyLength<&RenderStyle::paddingRight, &RenderStyle::setPaddingRight, &RenderStyle::initialPadding>::createHandler());
    setPropertyHandler(CSSPropertyPaddingBottom, ApplyPropertyLength<&RenderStyle::paddingBottom, &RenderStyle::setPaddingBottom, &RenderStyle::initialPadding>::createHandler());
    setPropertyHandler(CSSPropertyPaddingLeft, ApplyPropertyLength<&RenderStyle::paddingLeft, &RenderStyle::setPaddingLeft, &RenderStyle::initialPadding>::createHandler());
    setPropertyHandler(CSSPropertyPadding, ApplyPropertyExpanding<SuppressValue, CSSPropertyPaddingTop, CSSPropertyPaddingRight, CSSPropertyPaddingBottom, CSSPropertyPaddingLeft>::createHandler());

    setPropertyHandler(CSSPropertyWebkitPerspectiveOriginX, ApplyPropertyLength<&RenderStyle::perspectiveOriginX, &RenderStyle::setPerspectiveOriginX, &RenderStyle::initialPerspectiveOriginX>::createHandler());
    setPropertyHandler(CSSPropertyWebkitPerspectiveOriginY, ApplyPropertyLength<&RenderStyle::perspectiveOriginY, &RenderStyle::setPerspectiveOriginY, &RenderStyle::initialPerspectiveOriginY>::createHandler());
    setPropertyHandler(CSSPropertyWebkitPerspectiveOrigin, ApplyPropertyExpanding<SuppressValue, CSSPropertyWebkitPerspectiveOriginX, CSSPropertyWebkitPerspectiveOriginY>::createHandler());
    setPropertyHandler(CSSPropertyWebkitTransformOriginX, ApplyPropertyLength<&RenderStyle::transformOriginX, &RenderStyle::setTransformOriginX, &RenderStyle::initialTransformOriginX>::createHandler());
    setPropertyHandler(CSSPropertyWebkitTransformOriginY, ApplyPropertyLength<&RenderStyle::transformOriginY, &RenderStyle::setTransformOriginY, &RenderStyle::initialTransformOriginY>::createHandler());
    setPropertyHandler(CSSPropertyWebkitTransformOriginZ, ApplyPropertyComputeLength<float, &RenderStyle::transformOriginZ, &RenderStyle::setTransformOriginZ, &RenderStyle::initialTransformOriginZ>::createHandler());
    setPropertyHandler(CSSPropertyWebkitTransformOrigin, ApplyPropertyExpanding<SuppressValue, CSSPropertyWebkitTransformOriginX, CSSPropertyWebkitTransformOriginY, CSSPropertyWebkitTransformOriginZ>::createHandler());

    setPropertyHandler(CSSPropertyWebkitAnimationDelay, ApplyPropertyAnimation<double, &Animation::delay, &Animation::setDelay, &Animation::isDelaySet, &Animation::clearDelay, &Animation::initialAnimationDelay, &CSSStyleSelector::mapAnimationDelay, &RenderStyle::accessAnimations, &RenderStyle::animations>::createHandler());
    setPropertyHandler(CSSPropertyWebkitAnimationDirection, ApplyPropertyAnimation<Animation::AnimationDirection, &Animation::direction, &Animation::setDirection, &Animation::isDirectionSet, &Animation::clearDirection, &Animation::initialAnimationDirection, &CSSStyleSelector::mapAnimationDirection, &RenderStyle::accessAnimations, &RenderStyle::animations>::createHandler());
    setPropertyHandler(CSSPropertyWebkitAnimationDuration, ApplyPropertyAnimation<double, &Animation::duration, &Animation::setDuration, &Animation::isDurationSet, &Animation::clearDuration, &Animation::initialAnimationDuration, &CSSStyleSelector::mapAnimationDuration, &RenderStyle::accessAnimations, &RenderStyle::animations>::createHandler());
    setPropertyHandler(CSSPropertyWebkitAnimationFillMode, ApplyPropertyAnimation<unsigned, &Animation::fillMode, &Animation::setFillMode, &Animation::isFillModeSet, &Animation::clearFillMode, &Animation::initialAnimationFillMode, &CSSStyleSelector::mapAnimationFillMode, &RenderStyle::accessAnimations, &RenderStyle::animations>::createHandler());
    setPropertyHandler(CSSPropertyWebkitAnimationIterationCount, ApplyPropertyAnimation<int, &Animation::iterationCount, &Animation::setIterationCount, &Animation::isIterationCountSet, &Animation::clearIterationCount, &Animation::initialAnimationIterationCount, &CSSStyleSelector::mapAnimationIterationCount, &RenderStyle::accessAnimations, &RenderStyle::animations>::createHandler());
    setPropertyHandler(CSSPropertyWebkitAnimationName, ApplyPropertyAnimation<const String&, &Animation::name, &Animation::setName, &Animation::isNameSet, &Animation::clearName, &Animation::initialAnimationName, &CSSStyleSelector::mapAnimationName, &RenderStyle::accessAnimations, &RenderStyle::animations>::createHandler());
    setPropertyHandler(CSSPropertyWebkitAnimationPlayState, ApplyPropertyAnimation<EAnimPlayState, &Animation::playState, &Animation::setPlayState, &Animation::isPlayStateSet, &Animation::clearPlayState, &Animation::initialAnimationPlayState, &CSSStyleSelector::mapAnimationPlayState, &RenderStyle::accessAnimations, &RenderStyle::animations>::createHandler());
    setPropertyHandler(CSSPropertyWebkitAnimationTimingFunction, ApplyPropertyAnimation<const PassRefPtr<TimingFunction>, &Animation::timingFunction, &Animation::setTimingFunction, &Animation::isTimingFunctionSet, &Animation::clearTimingFunction, &Animation::initialAnimationTimingFunction, &CSSStyleSelector::mapAnimationTimingFunction, &RenderStyle::accessAnimations, &RenderStyle::animations>::createHandler());

    setPropertyHandler(CSSPropertyWebkitTransitionDelay, ApplyPropertyAnimation<double, &Animation::delay, &Animation::setDelay, &Animation::isDelaySet, &Animation::clearDelay, &Animation::initialAnimationDelay, &CSSStyleSelector::mapAnimationDelay, &RenderStyle::accessTransitions, &RenderStyle::transitions>::createHandler());
    setPropertyHandler(CSSPropertyWebkitTransitionDuration, ApplyPropertyAnimation<double, &Animation::duration, &Animation::setDuration, &Animation::isDurationSet, &Animation::clearDuration, &Animation::initialAnimationDuration, &CSSStyleSelector::mapAnimationDuration, &RenderStyle::accessTransitions, &RenderStyle::transitions>::createHandler());
    setPropertyHandler(CSSPropertyWebkitTransitionProperty, ApplyPropertyAnimation<int, &Animation::property, &Animation::setProperty, &Animation::isPropertySet, &Animation::clearProperty, &Animation::initialAnimationProperty, &CSSStyleSelector::mapAnimationProperty, &RenderStyle::accessTransitions, &RenderStyle::transitions>::createHandler());
    setPropertyHandler(CSSPropertyWebkitTransitionTimingFunction, ApplyPropertyAnimation<const PassRefPtr<TimingFunction>, &Animation::timingFunction, &Animation::setTimingFunction, &Animation::isTimingFunctionSet, &Animation::clearTimingFunction, &Animation::initialAnimationTimingFunction, &CSSStyleSelector::mapAnimationTimingFunction, &RenderStyle::accessTransitions, &RenderStyle::transitions>::createHandler());

    setPropertyHandler(CSSPropertyWebkitColumnCount, ApplyPropertyAuto<unsigned short, &RenderStyle::columnCount, &RenderStyle::setColumnCount, &RenderStyle::hasAutoColumnCount, &RenderStyle::setHasAutoColumnCount>::createHandler());
    setPropertyHandler(CSSPropertyWebkitColumnGap, ApplyPropertyAuto<float, &RenderStyle::columnGap, &RenderStyle::setColumnGap, &RenderStyle::hasNormalColumnGap, &RenderStyle::setHasNormalColumnGap, ComputeLength, CSSValueNormal>::createHandler());
    setPropertyHandler(CSSPropertyWebkitColumnWidth, ApplyPropertyAuto<float, &RenderStyle::columnWidth, &RenderStyle::setColumnWidth, &RenderStyle::hasAutoColumnWidth, &RenderStyle::setHasAutoColumnWidth, ComputeLength>::createHandler());
    setPropertyHandler(CSSPropertyWebkitColumns, ApplyPropertyExpanding<SuppressValue, CSSPropertyWebkitColumnWidth, CSSPropertyWebkitColumnCount>::createHandler());

    setPropertyHandler(CSSPropertyWebkitHighlight, ApplyPropertyString<MapNoneToNull, &RenderStyle::highlight, &RenderStyle::setHighlight, &RenderStyle::initialHighlight>::createHandler());
    setPropertyHandler(CSSPropertyWebkitHyphenateCharacter, ApplyPropertyString<MapAutoToNull, &RenderStyle::hyphenationString, &RenderStyle::setHyphenationString, &RenderStyle::initialHyphenationString>::createHandler());

    setPropertyHandler(CSSPropertyWebkitTextCombine, ApplyPropertyDefault<TextCombine, &RenderStyle::textCombine, TextCombine, &RenderStyle::setTextCombine, TextCombine, &RenderStyle::initialTextCombine>::createHandler());
    setPropertyHandler(CSSPropertyWebkitTextEmphasisPosition, ApplyPropertyDefault<TextEmphasisPosition, &RenderStyle::textEmphasisPosition, TextEmphasisPosition, &RenderStyle::setTextEmphasisPosition, TextEmphasisPosition, &RenderStyle::initialTextEmphasisPosition>::createHandler());
    setPropertyHandler(CSSPropertyWebkitTextEmphasisStyle, ApplyPropertyTextEmphasisStyle::createHandler());

    setPropertyHandler(CSSPropertyZIndex, ApplyPropertyAuto<int, &RenderStyle::zIndex, &RenderStyle::setZIndex, &RenderStyle::hasAutoZIndex, &RenderStyle::setHasAutoZIndex>::createHandler());
}


}
