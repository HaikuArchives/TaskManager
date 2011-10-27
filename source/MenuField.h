/*
 * Copyright 2000 by Thomas Krammer
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 *
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef TM_MENU_FIELD_H
#define TM_MENU_FIELD_H

float estimate_menu_field_width(const char *label, BMenu *menu);

BMenuField *create_menu_field(BPoint position, const char *name, const char *label, BMenu *menu, uint32 resizingMode=B_FOLLOW_LEFT | B_FOLLOW_TOP);

#endif // TM_MENU_FIELD_H