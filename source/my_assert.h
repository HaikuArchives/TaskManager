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

#ifndef MY_ASSERT_H
#define MY_ASSERT_H

// Simple BAlert based assert macro.
#define MY_ASSERT(_x_) \
	if(!(_x_)) { \
		char _buf_[255]; \
		sprintf(_buf_, "assertion failed\n%s\nLine: %d\n File:%s", #_x_, __LINE__, __FILE__); \
		BAlert *_alert_ = new BAlert("alert", _buf_, "OK", NULL, NULL, B_WIDTH_AS_USUAL, B_STOP_ALERT); \
		_alert_->Go(); \
		be_app->Quit(); \
	}

#define FAILED(_x_) \
	(_x_ < B_OK)
	
#define SUCCEEDED(_x_) \
	(_x_ >= B_OK)

#define RETURN_IF_FAILED(_x_) \
	{ \
		status_t __result__ = _x_; \
		if( FAILED(__result__) ) \
			return __result__; \
	}

#endif // MY_ASSERT_H