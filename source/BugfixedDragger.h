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
 
#ifndef BUGFIXED_DRAGGER_H
#define BUGFIXED_DRAGGER_H

class _EXPORT CBugfixedDragger : public BDragger
{
	public:
	CBugfixedDragger(BRect frame, BView *target, uint32 resizingMode=B_FOLLOW_NONE);
	CBugfixedDragger(BMessage *archive);

	static BArchivable *Instantiate(BMessage *archive);
	virtual	status_t Archive(BMessage *data, bool deep = true) const;
	
	virtual void AttachedToWindow();
	virtual void Draw(BRect updateRect);
	
	protected:
	rgb_color	bgColor;
};

#endif // BUGFIXED_DRAGGER_H