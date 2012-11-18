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
 
#ifndef TASKMGR_COMMON_H
#define TASKMGR_COMMON_H

// ====== Error Codes ======

const status_t E_CONTROLLED_BY_DEBUGGER		= 0x80005001;
const status_t E_NOT_HANDLED				= 0x80005002;

// ====== Update Speeds ======

extern const bigtime_t SLOW_PULSE_RATE;
extern const bigtime_t NORMAL_PULSE_RATE;
extern const bigtime_t FAST_PULSE_RATE;

// ====== Message Fields ======

// Common message fields shared by various messages
extern const char * const MESSAGE_DATA_ID_INDEX;					// int32
extern const char * const MESSAGE_DATA_ID_COLOR;					// rgb_color

#endif // TASKMGR_COMMON_H