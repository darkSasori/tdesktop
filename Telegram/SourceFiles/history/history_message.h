/*
This file is part of Telegram Desktop,
the official desktop version of Telegram messaging app, see https://telegram.org

Telegram Desktop is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

It is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

In addition, as a special exception, the copyright holders give permission
to link the code of portions of this program with the OpenSSL library.

Full license: https://github.com/telegramdesktop/tdesktop/blob/master/LICENSE
Copyright (c) 2014-2017 John Preston, https://desktop.telegram.org
*/
#pragma once

void historyInitMessages();

class HistoryMessage : public HistoryItem, private HistoryItemInstantiated<HistoryMessage> {
public:
	static gsl::not_null<HistoryMessage*> create(gsl::not_null<History*> history, const MTPDmessage &msg) {
		return _create(history, msg);
	}
	static gsl::not_null<HistoryMessage*> create(gsl::not_null<History*> history, const MTPDmessageService &msg) {
		return _create(history, msg);
	}
	static gsl::not_null<HistoryMessage*> create(gsl::not_null<History*> history, MsgId msgId, MTPDmessage::Flags flags, QDateTime date, int32 from, HistoryMessage *fwd) {
		return _create(history, msgId, flags, date, from, fwd);
	}
	static gsl::not_null<HistoryMessage*> create(gsl::not_null<History*> history, MsgId msgId, MTPDmessage::Flags flags, MsgId replyTo, int32 viaBotId, QDateTime date, int32 from, const TextWithEntities &textWithEntities) {
		return _create(history, msgId, flags, replyTo, viaBotId, date, from, textWithEntities);
	}
	static gsl::not_null<HistoryMessage*> create(gsl::not_null<History*> history, MsgId msgId, MTPDmessage::Flags flags, MsgId replyTo, int32 viaBotId, QDateTime date, int32 from, DocumentData *doc, const QString &caption, const MTPReplyMarkup &markup) {
		return _create(history, msgId, flags, replyTo, viaBotId, date, from, doc, caption, markup);
	}
	static gsl::not_null<HistoryMessage*> create(gsl::not_null<History*> history, MsgId msgId, MTPDmessage::Flags flags, MsgId replyTo, int32 viaBotId, QDateTime date, int32 from, PhotoData *photo, const QString &caption, const MTPReplyMarkup &markup) {
		return _create(history, msgId, flags, replyTo, viaBotId, date, from, photo, caption, markup);
	}
	static gsl::not_null<HistoryMessage*> create(gsl::not_null<History*> history, MsgId msgId, MTPDmessage::Flags flags, MsgId replyTo, int32 viaBotId, QDateTime date, int32 from, GameData *game, const MTPReplyMarkup &markup) {
		return _create(history, msgId, flags, replyTo, viaBotId, date, from, game, markup);
	}

	void initTime();
	void initMedia(const MTPMessageMedia *media);
	void initMediaFromDocument(DocumentData *doc, const QString &caption);
	void fromNameUpdated(int32 width) const;

	int32 plainMaxWidth() const;
	QRect countGeometry() const;

	bool drawBubble() const;
	bool hasBubble() const override {
		return drawBubble();
	}
	bool displayFromName() const {
		if (!hasFromName()) return false;
		if (isAttachedToPrevious()) return false;
		return true;
	}
	bool displayEditedBadge(bool hasViaBotOrInlineMarkup) const;
	bool uploading() const {
		return _media && _media->uploading();
	}

	void drawInfo(Painter &p, int32 right, int32 bottom, int32 width, bool selected, InfoDisplayType type) const override;
	void setViewsCount(int32 count) override;
	void setId(MsgId newId) override;
	void draw(Painter &p, QRect clip, TextSelection selection, TimeMs ms) const override;

	void dependencyItemRemoved(HistoryItem *dependency) override;

	bool hasPoint(QPoint point) const override;
	bool pointInTime(int right, int bottom, QPoint point, InfoDisplayType type) const override;

	HistoryTextState getState(QPoint point, HistoryStateRequest request) const override;
	void updatePressed(QPoint point) override;

	TextSelection adjustSelection(TextSelection selection, TextSelectType type) const override;

	// ClickHandlerHost interface
	void clickHandlerActiveChanged(const ClickHandlerPtr &p, bool active) override {
		if (_media) _media->clickHandlerActiveChanged(p, active);
		HistoryItem::clickHandlerActiveChanged(p, active);
	}
	void clickHandlerPressedChanged(const ClickHandlerPtr &p, bool pressed) override {
		if (_media) _media->clickHandlerPressedChanged(p, pressed);
		HistoryItem::clickHandlerPressedChanged(p, pressed);
	}

	QString notificationHeader() const override;

	void applyEdition(const MTPDmessage &message) override;
	void applyEdition(const MTPDmessageService &message) override;
	void updateMedia(const MTPMessageMedia *media) override;
	void updateReplyMarkup(const MTPReplyMarkup *markup) override {
		setReplyMarkup(markup);
	}
	int32 addToOverview(AddToOverviewMethod method) override;
	void eraseFromOverview() override;

	TextWithEntities selectedText(TextSelection selection) const override;
	void setText(const TextWithEntities &textWithEntities) override;
	TextWithEntities originalText() const override;
	bool textHasLinks() const override;

	int infoWidth() const override;
	int timeLeft() const override;
	int timeWidth() const override {
		return _timeWidth;
	}

	int viewsCount() const override {
		if (auto views = Get<HistoryMessageViews>()) {
			return views->_views;
		}
		return HistoryItem::viewsCount();
	}

	bool updateDependencyItem() override {
		if (auto reply = Get<HistoryMessageReply>()) {
			return reply->updateData(this, true);
		}
		return true;
	}
	MsgId dependencyMsgId() const override {
		return replyToId();
	}

	HistoryMessage *toHistoryMessage() override { // dynamic_cast optimize
		return this;
	}
	const HistoryMessage *toHistoryMessage() const override { // dynamic_cast optimize
		return this;
	}

	// hasFromPhoto() returns true even if we don't display the photo
	// but we need to skip a place at the left side for this photo
	bool displayFromPhoto() const;
	bool hasFromPhoto() const;

	~HistoryMessage();

private:
	HistoryMessage(gsl::not_null<History*> history, const MTPDmessage &msg);
	HistoryMessage(gsl::not_null<History*> history, const MTPDmessageService &msg);
	HistoryMessage(gsl::not_null<History*> history, MsgId msgId, MTPDmessage::Flags flags, QDateTime date, int32 from, HistoryMessage *fwd); // local forwarded
	HistoryMessage(gsl::not_null<History*> history, MsgId msgId, MTPDmessage::Flags flags, MsgId replyTo, int32 viaBotId, QDateTime date, int32 from, const TextWithEntities &textWithEntities); // local message
	HistoryMessage(gsl::not_null<History*> history, MsgId msgId, MTPDmessage::Flags flags, MsgId replyTo, int32 viaBotId, QDateTime date, int32 from, DocumentData *doc, const QString &caption, const MTPReplyMarkup &markup); // local document
	HistoryMessage(gsl::not_null<History*> history, MsgId msgId, MTPDmessage::Flags flags, MsgId replyTo, int32 viaBotId, QDateTime date, int32 from, PhotoData *photo, const QString &caption, const MTPReplyMarkup &markup); // local photo
	HistoryMessage(gsl::not_null<History*> history, MsgId msgId, MTPDmessage::Flags flags, MsgId replyTo, int32 viaBotId, QDateTime date, int32 from, GameData *game, const MTPReplyMarkup &markup); // local game
	friend class HistoryItemInstantiated<HistoryMessage>;

	void setEmptyText();

	// For an invoice button we replace the button text with a "Receipt" key.
	// It should show the receipt for the payed invoice. Still let mobile apps do that.
	void replaceBuyWithReceiptInMarkup();

	void initDimensions() override;
	int resizeContentGetHeight() override;
	int performResizeGetHeight();
	void applyEditionToEmpty();

	bool displayForwardedFrom() const {
		if (auto forwarded = Get<HistoryMessageForwarded>()) {
			return Has<HistoryMessageVia>()
				|| !_media
				|| !_media->isDisplayed()
				|| !_media->hideForwardedFrom()
				|| forwarded->_authorOriginal->isChannel();
		}
		return false;
	}
	void paintFromName(Painter &p, QRect &trect, bool selected) const;
	void paintForwardedInfo(Painter &p, QRect &trect, bool selected) const;
	void paintReplyInfo(Painter &p, QRect &trect, bool selected) const;
	// this method draws "via @bot" if it is not painted in forwarded info or in from name
	void paintViaBotIdInfo(Painter &p, QRect &trect, bool selected) const;
	void paintText(Painter &p, QRect &trect, TextSelection selection) const;

	bool getStateFromName(QPoint point, QRect &trect, HistoryTextState *outResult) const;
	bool getStateForwardedInfo(QPoint point, QRect &trect, HistoryTextState *outResult, const HistoryStateRequest &request) const;
	bool getStateReplyInfo(QPoint point, QRect &trect, HistoryTextState *outResult) const;
	bool getStateViaBotIdInfo(QPoint point, QRect &trect, HistoryTextState *outResult) const;
	bool getStateText(QPoint point, QRect &trect, HistoryTextState *outResult, const HistoryStateRequest &request) const;

	void setMedia(const MTPMessageMedia *media);
	void setReplyMarkup(const MTPReplyMarkup *markup);

	QString _timeText;
	int _timeWidth = 0;

	struct CreateConfig {
		MsgId replyTo = 0;
		UserId viaBotId = 0;
		int viewsCount = -1;
		PeerId authorIdOriginal = 0;
		PeerId fromIdOriginal = 0;
		MsgId originalId = 0;
		QDateTime originalDate;
		QDateTime editDate;

		// For messages created from MTP structs.
		const MTPReplyMarkup *mtpMarkup = nullptr;

		// For messages created from existing messages (forwarded).
		const HistoryMessageReplyMarkup *inlineMarkup = nullptr;
	};
	void createComponentsHelper(MTPDmessage::Flags flags, MsgId replyTo, int32 viaBotId, const MTPReplyMarkup &markup);
	void createComponents(const CreateConfig &config);

	class KeyboardStyle : public ReplyKeyboard::Style {
	public:
		using ReplyKeyboard::Style::Style;

		int buttonRadius() const override;

		void startPaint(Painter &p) const override;
		const style::TextStyle &textStyle() const override;
		void repaint(gsl::not_null<const HistoryItem*> item) const override;

	protected:
		void paintButtonBg(Painter &p, const QRect &rect, float64 howMuchOver) const override;
		void paintButtonIcon(Painter &p, const QRect &rect, int outerWidth, HistoryMessageReplyMarkup::Button::Type type) const override;
		void paintButtonLoading(Painter &p, const QRect &rect) const override;
		int minButtonWidth(HistoryMessageReplyMarkup::Button::Type type) const override;

	};

	void updateMediaInBubbleState();

};

inline MTPDmessage::Flags newMessageFlags(PeerData *p) {
	MTPDmessage::Flags result = 0;
	if (!p->isSelf()) {
		result |= MTPDmessage::Flag::f_out;
		//if (p->isChat() || (p->isUser() && !p->asUser()->botInfo)) {
		//	result |= MTPDmessage::Flag::f_unread;
		//}
	}
	return result;
}

struct HistoryServiceDependentData {
	MsgId msgId = 0;
	HistoryItem *msg = nullptr;
	ClickHandlerPtr lnk;
};

struct HistoryServicePinned : public RuntimeComponent<HistoryServicePinned>, public HistoryServiceDependentData {
};

struct HistoryServiceGameScore : public RuntimeComponent<HistoryServiceGameScore>, public HistoryServiceDependentData {
	int score = 0;
};

struct HistoryServicePayment : public RuntimeComponent<HistoryServicePayment>, public HistoryServiceDependentData {
	QString amount;
};

namespace HistoryLayout {
class ServiceMessagePainter;
} // namespace HistoryLayout

class HistoryService : public HistoryItem, private HistoryItemInstantiated<HistoryService> {
public:
	struct PreparedText {
		QString text;
		QList<ClickHandlerPtr> links;
	};

	static gsl::not_null<HistoryService*> create(gsl::not_null<History*> history, const MTPDmessageService &message) {
		return _create(history, message);
	}
	static gsl::not_null<HistoryService*> create(gsl::not_null<History*> history, MsgId msgId, QDateTime date, const PreparedText &message, MTPDmessage::Flags flags = 0, UserId from = 0, PhotoData *photo = nullptr) {
		return _create(history, msgId, date, message, flags, from, photo);
	}

	bool updateDependencyItem() override;
	MsgId dependencyMsgId() const override {
		if (auto dependent = GetDependentData()) {
			return dependent->msgId;
		}
		return 0;
	}
	bool notificationReady() const override {
		if (auto dependent = GetDependentData()) {
			return (dependent->msg || !dependent->msgId);
		}
		return true;
	}

	QRect countGeometry() const;

	void draw(Painter &p, QRect clip, TextSelection selection, TimeMs ms) const override;
	bool hasPoint(QPoint point) const override;
	HistoryTextState getState(QPoint point, HistoryStateRequest request) const override;

	TextSelection adjustSelection(TextSelection selection, TextSelectType type) const override {
		return _text.adjustSelection(selection, type);
	}

	void clickHandlerActiveChanged(const ClickHandlerPtr &p, bool active) override {
		if (_media) _media->clickHandlerActiveChanged(p, active);
		HistoryItem::clickHandlerActiveChanged(p, active);
	}
	void clickHandlerPressedChanged(const ClickHandlerPtr &p, bool pressed) override {
		if (_media) _media->clickHandlerPressedChanged(p, pressed);
		HistoryItem::clickHandlerPressedChanged(p, pressed);
	}

	void applyEdition(const MTPDmessageService &message) override;

	int32 addToOverview(AddToOverviewMethod method) override;
	void eraseFromOverview() override;

	bool needCheck() const override {
		return false;
	}
	bool serviceMsg() const override {
		return true;
	}
	TextWithEntities selectedText(TextSelection selection) const override;
	QString inDialogsText() const override;
	QString inReplyText() const override;

	~HistoryService();

protected:
	friend class HistoryLayout::ServiceMessagePainter;

	HistoryService(gsl::not_null<History*> history, const MTPDmessageService &message);
	HistoryService(gsl::not_null<History*> history, MsgId msgId, QDateTime date, const PreparedText &message, MTPDmessage::Flags flags = 0, UserId from = 0, PhotoData *photo = 0);
	friend class HistoryItemInstantiated<HistoryService>;

	void initDimensions() override;
	int resizeContentGetHeight() override;

	void setServiceText(const PreparedText &prepared);

	QString fromLinkText() const {
		return textcmdLink(1, _from->name);
	};
	ClickHandlerPtr fromLink() const {
		return peerOpenClickHandler(_from);
	};

	void removeMedia();

private:
	HistoryServiceDependentData *GetDependentData() {
		if (auto pinned = Get<HistoryServicePinned>()) {
			return pinned;
		} else if (auto gamescore = Get<HistoryServiceGameScore>()) {
			return gamescore;
		} else if (auto payment = Get<HistoryServicePayment>()) {
			return payment;
		}
		return nullptr;
	}
	const HistoryServiceDependentData *GetDependentData() const {
		return const_cast<HistoryService*>(this)->GetDependentData();
	}
	bool updateDependent(bool force = false);
	void updateDependentText();
	void clearDependency();

	void createFromMtp(const MTPDmessageService &message);
	void setMessageByAction(const MTPmessageAction &action);

	PreparedText preparePinnedText();
	PreparedText prepareGameScoreText();
	PreparedText preparePaymentSentText();

};

class HistoryJoined : public HistoryService, private HistoryItemInstantiated<HistoryJoined> {
public:
	static gsl::not_null<HistoryJoined*> create(gsl::not_null<History*> history, const QDateTime &inviteDate, gsl::not_null<UserData*> inviter, MTPDmessage::Flags flags) {
		return _create(history, inviteDate, inviter, flags);
	}

protected:
	HistoryJoined(gsl::not_null<History*> history, const QDateTime &inviteDate, gsl::not_null<UserData*> inviter, MTPDmessage::Flags flags);
	using HistoryItemInstantiated<HistoryJoined>::_create;
	friend class HistoryItemInstantiated<HistoryJoined>;

private:
	static PreparedText GenerateText(gsl::not_null<History*> history, gsl::not_null<UserData*> inviter);

};
