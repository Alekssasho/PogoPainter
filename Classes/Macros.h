//
//  Macros.h
//  PogoPainter
//
//  Created by Aleksandar Angelov on 11/28/14.
//
//

#ifndef PogoPainter_Macros_h
#define PogoPainter_Macros_h

#define SEND_EVENT(name, userdata)  Director::getInstance()->getEventDispatcher()->dispatchCustomEvent((name), (userdata))

#define ADD_DELEGATE(name, callback) Director::getInstance()->getEventDispatcher()->addCustomEventListener((name), (callback))

#endif
