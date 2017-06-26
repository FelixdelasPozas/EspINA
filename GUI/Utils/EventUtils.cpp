/*

 Copyright (C) 2017 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

 This file is part of ESPINA.

 ESPINA is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 
 */

// ESPINA
#include "EventUtils.h"

// Qt
#include <QEvent>
#include <QString>

//--------------------------------------------------------------------
QString ESPINA::GUI::Utils::toString(QEvent* e)
{
  QString returnValue("Unknown event");

  switch(e->type())
  {
    case 0: returnValue = "Qt::None - invalid event"; break;
    case 1: returnValue = "Qt::Timer - timer event"; break;
    case 2: returnValue = "Qt::MouseButtonPress - mouse button pressed"; break;
    case 3: returnValue = "Qt::MouseButtonRelease - mouse button released"; break;
    case 4: returnValue = "Qt::MouseButtonDblClick - mouse button double click"; break;
    case 5: returnValue = "Qt::MouseMove - mouse move"; break;
    case 6: returnValue = "Qt::KeyPress - key pressed"; break;
    case 7: returnValue = "Qt::KeyRelease - key released"; break;
    case 8: returnValue = "Qt::FocusIn - keyboard focus received"; break;
    case 9: returnValue = "Qt::FocusOut - keyboard focus lost"; break;
    case 10: returnValue = "Qt::Enter - mouse enters widget"; break;
    case 11: returnValue = "Qt::Leave - mouse leaves widget"; break;
    case 12: returnValue = "Qt::Paint - paint widget"; break;
    case 13: returnValue = "Qt::Move - move widget"; break;
    case 14: returnValue = "Qt::Resize - resize widget"; break;
    case 15: returnValue = "Qt::Create - after widget creation"; break;
    case 16: returnValue = "Qt::Destroy - during widget destruction"; break;
    case 17: returnValue = "Qt::Show - widget is shown"; break;
    case 18: returnValue = "Qt::Hide - widget is hidden"; break;
    case 19: returnValue = "Qt::Close - request to close widget"; break;
    case 20: returnValue = "Qt::Quit - request to quit application"; break;
    case 21: returnValue = "Qt::ParentChange - widget has been reparented"; break;
    case 131: returnValue = "Qt::ParentAboutToChange - sent just before the parent change is done"; break;
    case 22: returnValue = "Qt::ThreadChange - object has changed threads"; break;
    case 24: returnValue = "Qt::WindowActivate - window was activated"; break;
    case 25: returnValue = "Qt::WindowDeactivate - window was deactivated"; break;
    case 26: returnValue = "Qt::ShowToParent - widget is shown to parent"; break;
    case 27: returnValue = "Qt::HideToParent - widget is hidden to parent"; break;
    case 31: returnValue = "Qt::Wheel - wheel event"; break;
    case 33: returnValue = "Qt::WindowTitleChange - window title changed"; break;
    case 34: returnValue = "Qt::WindowIconChange - icon changed"; break;
    case 35: returnValue = "Qt::ApplicationWindowIconChange - application icon changed"; break;
    case 36: returnValue = "Qt::ApplicationFontChange - application font changed"; break;
    case 37: returnValue = "Qt::ApplicationLayoutDirectionChange - application layout direction changed"; break;
    case 38: returnValue = "Qt::ApplicationPaletteChange - application palette changed"; break;
    case 39: returnValue = "Qt::PaletteChange - widget palette changed"; break;
    case 40: returnValue = "Qt::Clipboard - internal clipboard event"; break;
    case 42: returnValue = "Qt::Speech - reserved for speech input"; break;
    case 43: returnValue = "Qt::MetaCall - meta call event"; break;
    case 50: returnValue = "Qt::SockAct - socket activation"; break;
    case 132: returnValue = "Qt::WinEventAct - win event activation"; break;
    case 52: returnValue = "Qt::DeferredDelete - deferred delete event"; break;
    case 60: returnValue = "Qt::DragEnter - drag moves into widget"; break;
    case 61: returnValue = "Qt::DragMove - drag moves in widget"; break;
    case 62: returnValue = "Qt::DragLeave - drag leaves or is cancelled"; break;
    case 63: returnValue = "Qt::Drop - actual drop"; break;
    case 64: returnValue = "Qt::DragResponse - drag accepted/rejected"; break;
    case 68: returnValue = "Qt::ChildAdded - new child widget"; break;
    case 69: returnValue = "Qt::ChildPolished - polished child widget"; break;
    case 71: returnValue = "Qt::ChildRemoved - deleted child widget"; break;
    case 73: returnValue = "Qt::ShowWindowRequest - widget's window should be mapped"; break;
    case 74: returnValue = "Qt::PolishRequest - widget should be polished"; break;
    case 75: returnValue = "Qt::Polish - widget is polished"; break;
    case 76: returnValue = "Qt::LayoutRequest - widget should be relayouted"; break;
    case 77: returnValue = "Qt::UpdateRequest - widget should be repainted"; break;
    case 78: returnValue = "Qt::UpdateLater - request update() later"; break;
    case 79: returnValue = "Qt::EmbeddingControl - ActiveX embedding"; break;
    case 80: returnValue = "Qt::ActivateControl - ActiveX activation"; break;
    case 81: returnValue = "Qt::DeactivateControl - ActiveX deactivation"; break;
    case 82: returnValue = "Qt::ContextMenu - context popup menu"; break;
    case 83: returnValue = "Qt::InputMethod - input method"; break;
    case 86: returnValue = "Qt::AccessibilityPrepare - accessibility information is requested"; break;
    case 87: returnValue = "Qt::TabletMove - Wacom tablet event"; break;
    case 88: returnValue = "Qt::LocaleChange - the system locale changed"; break;
    case 89: returnValue = "Qt::LanguageChange - the application language changed"; break;
    case 90: returnValue = "Qt::LayoutDirectionChange - the layout direction changed"; break;
    case 91: returnValue = "Qt::Style - internal style event"; break;
    case 92: returnValue = "Qt::TabletPress - tablet press"; break;
    case 93: returnValue = "Qt::TabletRelease - tablet release"; break;
    case 94: returnValue = "Qt::OkRequest - CE (Ok) button pressed"; break;
    case 95: returnValue = "Qt::HelpRequest - CE (?)  button pressed"; break;
    case 96: returnValue = "Qt::IconDrag - proxy icon dragged"; break;
    case 97: returnValue = "Qt::FontChange - font has changed"; break;
    case 98: returnValue = "Qt::EnabledChange - enabled state has changed"; break;
    case 99: returnValue = "Qt::ActivationChange - window activation has changed"; break;
    case 100: returnValue = "Qt::StyleChange - style has changed"; break;
    case 101: returnValue = "Qt::IconTextChange - icon text has changed"; break;
    case 102: returnValue = "Qt::ModifiedChange - modified state has changed"; break;
    case 109: returnValue = "Qt::MouseTrackingChange - mouse tracking state has changed"; break;
    case 103: returnValue = "Qt::WindowBlocked - window is about to be blocked modally"; break;
    case 104: returnValue = "Qt::WindowUnblocked - windows modal blocking has ended"; break;
    case 105: returnValue = "Qt::WindowStateChange"; break;
    case 110: returnValue = "Qt::ToolTip"; break;
    case 111: returnValue = "Qt::WhatsThis"; break;
    case 112: returnValue = "Qt::StatusTip"; break;
    case 113: returnValue = "Qt::ActionChanged"; break;
    case 114: returnValue = "Qt::ActionAdded"; break;
    case 115: returnValue = "Qt::ActionRemoved"; break;
    case 116: returnValue = "Qt::FileOpen - file open request"; break;
    case 117: returnValue = "Qt::Shortcut - shortcut triggered"; break;
    case 51: returnValue = "Qt::ShortcutOverride - shortcut override request"; break;
    case 118: returnValue = "Qt::WhatsThisClicked"; break;
    case 120: returnValue = "Qt::ToolBarChange - toolbar visibility toggled"; break;
    case 121: returnValue = "Qt::ApplicationActivate - application has been changed to active"; break;
    case 122: returnValue = "Qt::ApplicationDeactivate - application has been changed to inactive"; break;
    case 123: returnValue = "Qt::QueryWhatsThis - query what's this widget help"; break;
    case 124: returnValue = "Qt::EnterWhatsThisMode"; break;
    case 125: returnValue = "Qt::LeaveWhatsThisMode"; break;
    case 126: returnValue = "Qt::ZOrderChange - child widget has had its z-order changed"; break;
    case 127: returnValue = "Qt::HoverEnter - mouse cursor enters a hover widget"; break;
    case 128: returnValue = "Qt::HoverLeave - mouse cursor leaves a hover widget"; break;
    case 129: returnValue = "Qt::HoverMove - mouse cursor move inside a hover widget"; break;
    case 119: returnValue = "Qt::AccessibilityHelp - accessibility help text request"; break;
    case 130: returnValue = "Qt::AccessibilityDescription - accessibility description text request"; break;
    case 152: returnValue = "Qt::AcceptDropsChange"; break;
    case 153: returnValue = "Qt::MenubarUpdated - Support event for Q3MainWindow: which needs to knwow when QMenubar is updated."; break;
    case 154: returnValue = "Qt::ZeroTimerEvent - Used for Windows Zero timer events"; break;
    case 155: returnValue = "Qt::GraphicsSceneMouseMove - GraphicsView"; break;
    case 156: returnValue = "Qt::GraphicsSceneMousePress - GraphicsView"; break;
    case 157: returnValue = "Qt::GraphicsSceneMouseRelease - GraphicsView"; break;
    case 158: returnValue = "Qt::GraphicsSceneMouseDoubleClick - GraphicsView"; break;
    case 159: returnValue = "Qt::GraphicsSceneContextMenu - GraphicsView"; break;
    case 160: returnValue = "Qt::GraphicsSceneHoverEnter - GraphicsView"; break;
    case 161: returnValue = "Qt::GraphicsSceneHoverMove - GraphicsView"; break;
    case 162: returnValue = "Qt::GraphicsSceneHoverLeave - GraphicsView"; break;
    case 163: returnValue = "Qt::GraphicsSceneHelp - GraphicsView"; break;
    case 164: returnValue = "Qt::GraphicsSceneDragEnter - GraphicsView"; break;
    case 165: returnValue = "Qt::GraphicsSceneDragMove - GraphicsView"; break;
    case 166: returnValue = "Qt::GraphicsSceneDragLeave - GraphicsView"; break;
    case 167: returnValue = "Qt::GraphicsSceneDrop - GraphicsView"; break;
    case 168: returnValue = "Qt::GraphicsSceneWheel - GraphicsView"; break;
    case 169: returnValue = "Qt::KeyboardLayoutChange - keyboard layout changed"; break;
    case 170: returnValue = "Qt::DynamicPropertyChange - A dynamic property was changed through setProperty/property"; break;
    case 171: returnValue = "Qt::TabletEnterProximity"; break;
    case 172: returnValue = "Qt::TabletLeaveProximity"; break;
    case 173: returnValue = "Qt::NonClientAreaMouseMove"; break;
    case 174: returnValue = "Qt::NonClientAreaMouseButtonPress"; break;
    case 175: returnValue = "Qt::NonClientAreaMouseButtonRelease"; break;
    case 176: returnValue = "Qt::NonClientAreaMouseButtonDblClick"; break;
    case 177: returnValue = "Qt::MacSizeChange - when the Qt::WA_Mac{Normal:Small:Mini}Size changes"; break;
    case 178: returnValue = "Qt::ContentsRectChange - sent by QWidget::setContentsMargins (internal)"; break;
    case 179: returnValue = "Qt::MacGLWindowChange - Internal! the window of the GLWidget has changed"; break;
    case 180: returnValue = "Qt::FutureCallOut"; break;
    case 181: returnValue = "Qt::GraphicsSceneResize"; break;
    case 182: returnValue = "Qt::GraphicsSceneMove"; break;
    case 183: returnValue = "Qt::CursorChange"; break;
    case 184: returnValue = "Qt::ToolTipChange"; break;
    case 185: returnValue = "Qt::NetworkReplyUpdated - Internal for QNetworkReply"; break;
    case 186: returnValue = "Qt::GrabMouse"; break;
    case 187: returnValue = "Qt::UngrabMouse"; break;
    case 188: returnValue = "Qt::GrabKeyboard"; break;
    case 189: returnValue = "Qt::UngrabKeyboard"; break;
    case 191: returnValue = "Qt::MacGLClearDrawable - Internal Cocoa: the window has changed: so we must clear"; break;

    case 192: returnValue = "Qt::StateMachineSignal"; break;
    case 193: returnValue = "Qt::StateMachineWrapped"; break;
    case 194: returnValue = "Qt::TouchBegin"; break;
    case 195: returnValue = "Qt::TouchUpdate"; break;
    case 196: returnValue = "Qt::TouchEnd"; break;
    case 197: returnValue = "Qt::NativeGesture - Internal for platform gesture support"; break;
    case 198: returnValue = "Qt::Gesture"; break;
    case 199: returnValue = "Qt::RequestSoftwareInputPanel"; break;
    case 200: returnValue = "Qt::CloseSoftwareInputPanel"; break;
    case 201: returnValue = "Qt::UpdateSoftKeys - Internal for compressing soft key updates"; break;

    case 202: returnValue = "Qt::GestureOverride"; break;
    case 203: returnValue = "Qt::WinIdChange"; break;
    case 212: returnValue = "Qt::PlatformPanel"; break;

    case 1000: returnValue = "Qt::User - first user event id"; break;
    case 65535: returnValue = "Qt::MaxUser - last user event id"; break;
    default:
      returnValue = "UknownEvent";
      break;
  }

  return returnValue;
}
