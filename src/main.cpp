/**
 * Include the Geode headers.
 */
#include <Geode/Geode.hpp>
#include <Geode/modify/LevelSearchLayer.hpp>
#include <Geode/ui/BasedButtonSprite.hpp>

/**
 * Brings cocos2d and all Geode namespaces to the current scope.
 */
using namespace geode::prelude;

constexpr ccColor4B bronzeColor = {205, 127, 50, 255};
constexpr ccColor4B silverColor = {192, 192, 192, 255};
constexpr ccColor4B goldColor = {255, 215, 0, 255};
constexpr ccColor4B emeraldColor = {6, 182, 0, 255};
constexpr ccColor4B rubyColor = {255, 0, 106, 255};
constexpr ccColor4B diamondColor = {0, 140, 255, 255};
constexpr ccColor4B amethystColor = {242, 140, 255, 255};
constexpr ccColor4B legendColor = {0, 0, 0, 255};
constexpr ccColor4B defaultColor = {50, 50, 50, 255};

class TierCell : public CCNode
{
public:
	static TierCell *create(const std::string &tierName, int completed, int total, CCObject *target, SEL_MenuHandler selector)
	{
		auto ret = new TierCell();
		if (ret && ret->init(tierName, completed, total, target, selector))
		{
			ret->autorelease();
			return ret;
		}
		delete ret;
		return nullptr;
	}

	bool init(const std::string &tierName, int completed, int total, CCObject *target, SEL_MenuHandler selector)
	{
		if (!CCNode::init())
			return false;

		setContentSize({356.0f, 40.0f});

		// Background color based on tier
		ccColor4B bgColor;
		if (tierName == "Bronze")
		{
			bgColor = bronzeColor;
		}
		else if (tierName == "Silver")
		{
			bgColor = silverColor;
		}
		else if (tierName == "Gold")
		{
			bgColor = goldColor;
		}
		else if (tierName == "Emerald")
		{
			bgColor = emeraldColor;
		}
		else if (tierName == "Ruby")
		{
			bgColor = rubyColor;
		}
		else if (tierName == "Diamond")
		{
			bgColor = diamondColor;
		}
		else if (tierName == "Amethyst")
		{
			bgColor = amethystColor;
		}
		else if (tierName == "Legend")
		{
			bgColor = legendColor;
		}
		else
		{
			bgColor = defaultColor;
		}

		auto background = CCLayerColor::create(bgColor, 356.0f, 40.0f);
		addChild(background);

		// Tier name label
		auto label = CCLabelBMFont::create(tierName.c_str(), "bigFont.fnt");
		label->setPosition({20, 20});
		label->setAnchorPoint({0, 0.5f});
		label->setScale(0.6f);
		addChild(label);

		// Progress bar background
		auto barBG = CCLayerColor::create({30, 30, 30, 255}, 120, 16);
		barBG->setPosition({140, 12});
		addChild(barBG);

		// Progress bar fill
		float progress = total > 0 ? (float)completed / total : 0;
		auto barFill = CCLayerColor::create({0, 255, 0, 255}, 120 * progress, 16);
		barFill->setPosition({140, 12});
		addChild(barFill);

		// Progress text
		auto progressText = CCLabelBMFont::create(fmt::format("{}/{}", completed, total).c_str(), "goldFont.fnt");
		progressText->setPosition({200, 20});
		progressText->setScale(0.4f);
		addChild(progressText);

		// View button
		auto menu = CCMenu::create();
		menu->setPosition({0, 0});
		auto viewBtn = CCMenuItemSpriteExtra::create(
			ButtonSprite::create("View", "goldFont.fnt", "GJ_button_01.png", 0.7f),
			target, selector);
		viewBtn->setPosition({310, 20});
		viewBtn->setUserObject(CCString::create(tierName));
		menu->addChild(viewBtn);
		addChild(menu);

		return true;
	}
};

class TierLevelsLayer : public CCLayer, LevelManagerDelegate
{
public:
	static TierLevelsLayer *create(const std::string &tierName, const std::vector<std::string> &levelIds)
	{
		auto ret = new TierLevelsLayer();
		if (ret->init(tierName, levelIds))
		{
			ret->autorelease();
			return ret;
		}
		delete ret;
		return nullptr;
	}

	static CCScene *scene(const std::string &tierName, const std::vector<std::string> &levelIds)
	{
		auto scene = CCScene::create();
		scene->addChild(TierLevelsLayer::create(tierName, levelIds));
		return scene;
	}

	bool init(const std::string &tierName, const std::vector<std::string> &levelIds)
	{
		if (!CCLayer::init())
			return false;

		m_tierName = tierName;
		m_allLevelIds = levelIds;

		auto winSize = CCDirector::get()->getWinSize();

		// Background
		auto bg = CCSprite::create("GJ_gradientBG.png");
		bg->setAnchorPoint({0, 0});
		bg->setScaleX(winSize.width / bg->getTextureRect().size.width);
		bg->setScaleY(winSize.height / bg->getTextureRect().size.height);
		bg->setColor({34, 0, 0});
		addChild(bg);

		// Buttons
		auto menu = CCMenu::create();
		menu->setPosition({0, 0});

		auto backBtn = CCMenuItemSpriteExtra::create(
			CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png"),
			this,
			menu_selector(TierLevelsLayer::onBack));
		backBtn->setPosition({25, winSize.height - 25});
		menu->addChild(backBtn);

		auto leftBtn = CCMenuItemSpriteExtra::create(
			CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png"),
			this,
			menu_selector(TierLevelsLayer::onPrevPage));
		leftBtn->setPosition({24, winSize.height / 2});
		menu->addChild(leftBtn);

		auto rightBtnSprite = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
		rightBtnSprite->setFlipX(true);
		auto rightBtn = CCMenuItemSpriteExtra::create(
			rightBtnSprite,
			this, menu_selector(TierLevelsLayer::onNextPage));
		rightBtn->setPosition({winSize.width - 24, winSize.height / 2});
		menu->addChild(rightBtn);

		addChild(menu);
		setKeypadEnabled(true);

		loadPage(0);
		return true;
	}

	void loadPage(int page)
	{
		if (!m_loadingCircle)
		{
			m_loadingCircle = LoadingCircle::create();
			m_loadingCircle->setParentLayer(this);
		}
		m_loadingCircle->show();

		m_currentPage = page;

		int start = page * 10;
		int end = std::min(start + 10, (int)m_allLevelIds.size());

		std::string idsToFetch;
		for (int i = start; i < end; i++)
		{
			if (i > start)
				idsToFetch += ",";
			idsToFetch += m_allLevelIds[i];
		}

		auto glm = GameLevelManager::get();
		glm->m_levelManagerDelegate = this;

		auto searchObj = GJSearchObject::create(SearchType::Type19);
		searchObj->m_searchQuery = idsToFetch;
		glm->getOnlineLevels(searchObj);
	}

	void loadLevelsFinished(CCArray *levels, const char *, int) override
	{
		if (m_loadingCircle)
		{
			m_loadingCircle->fadeAndRemove();
			m_loadingCircle = nullptr;
		}

		if (m_listLayer)
			m_listLayer->removeFromParent();

		auto winSize = CCDirector::get()->getWinSize();

		auto listView = CustomListView::create(levels, BoomListType::Level, 190.0f, 356.0f);

		m_listLayer = GJListLayer::create(listView, m_tierName.c_str(), {0, 0, 0, 180}, 356.f, 220.f, 0);
		m_listLayer->setPosition(winSize / 2 - m_listLayer->getContentSize() / 2);

		// Cover the black bar
		auto textBar = CCLayerColor::create({34, 0, 0, 255}, 356.0f, 30.0f);
		textBar->setPosition({0.0f, 190.0f});
		m_listLayer->addChild(textBar);

		// Determine required completions based on tier
		int requiredCompletions = 0;
		if (m_tierName == "Bronze")
			requiredCompletions = 9;
		else if (m_tierName == "Silver")
			requiredCompletions = 13;
		else if (m_tierName == "Gold")
			requiredCompletions = 7;
		else if (m_tierName == "Emerald")
			requiredCompletions = 5;
		else if (m_tierName == "Ruby")
			requiredCompletions = 4;
		else if (m_tierName == "Diamond")
			requiredCompletions = 3;
		else if (m_tierName == "Amethyst")
			requiredCompletions = 2;
		else if (m_tierName == "Legend")
			requiredCompletions = 1;

		auto infoText = CCLabelBMFont::create(
			fmt::format("Clear any {} to unlock next tier", requiredCompletions).c_str(),
			"goldFont.fnt");
		infoText->setPosition({178, 205}); // center of the bar
		infoText->setScale(0.4f);
		m_listLayer->addChild(infoText);

		addChild(m_listLayer);
	}

	void loadLevelsFailed(const char *, int) override
	{
		if (m_loadingCircle)
		{
			m_loadingCircle->fadeAndRemove();
			m_loadingCircle = nullptr;
		}

		FLAlertLayer::create("Error", "Failed to load levels", "OK")->show();
	}

	void onPrevPage(CCObject *)
	{
		if (m_currentPage > 0)
			loadPage(m_currentPage - 1);
	}

	void onNextPage(CCObject *)
	{
		int maxPage = (m_allLevelIds.size() - 1) / 10;
		if (m_currentPage < maxPage)
			loadPage(m_currentPage + 1);
	}

	void onBack(CCObject *)
	{
		// Cancel pending requests
		auto glm = GameLevelManager::get();
		if (glm->m_levelManagerDelegate == this)
		{
			glm->m_levelManagerDelegate = nullptr;
		}
		
		if (m_loadingCircle)
		{
			m_loadingCircle->fadeAndRemove();
			m_loadingCircle = nullptr;
		}

		CCDirector::get()->popSceneWithTransition(0.5f, kPopTransitionFade);
	}

	void keyBackClicked() override
	{
		onBack(nullptr);
	}

private:
	std::string m_tierName;
	std::vector<std::string> m_allLevelIds;
	GJListLayer *m_listLayer = nullptr;
	int m_currentPage = 0;
	LoadingCircle *m_loadingCircle = nullptr;
};

CCNode *createLockedCell(const std::string &tierName, int requiredCompletions, const std::string &prevTierName)
{
	auto cell = CCNode::create();
	cell->setContentSize({356.0f, 40.0f});

	// Background
	ccColor4B bgColor;
	if (tierName == "Bronze")
	{
		bgColor = bronzeColor;
	}
	else if (tierName == "Silver")
	{
		bgColor = silverColor;
	}
	else if (tierName == "Gold")
	{
		bgColor = goldColor;
	}
	else if (tierName == "Emerald")
	{
		bgColor = emeraldColor;
	}
	else if (tierName == "Ruby")
	{
		bgColor = rubyColor;
	}
	else if (tierName == "Diamond")
	{
		bgColor = diamondColor;
	}
	else if (tierName == "Amethyst")
	{
		bgColor = amethystColor;
	}
	else if (tierName == "Legend")
	{
		bgColor = legendColor;
	}
	else
	{
		bgColor = defaultColor;
	}

	auto background = CCLayerColor::create(bgColor, 356.0f, 40.0f);
	cell->addChild(background);

	// Tier name
	auto label = CCLabelBMFont::create(tierName.c_str(), "bigFont.fnt");
	label->setPosition({20, 20});
	label->setAnchorPoint({0, 0.5f});
	label->setScale(0.6f);
	cell->addChild(label);

	// Requirement text
	auto reqText = CCLabelBMFont::create(
		fmt::format("Clear {} {} levels", requiredCompletions, prevTierName).c_str(),
		"goldFont.fnt");
	reqText->setPosition({200, 20});
	reqText->setScale(0.4f);
	cell->addChild(reqText);

	// Lock icon
	auto lockSprite = CCSprite::createWithSpriteFrameName("GJ_lock_001.png");
	lockSprite->setPosition({310, 20});
	lockSprite->setScale(0.8f);
	cell->addChild(lockSprite);

	return cell;
}

class WSRLayer : public CCLayer
{
public:
	static WSRLayer *create()
	{
		auto ret = new WSRLayer();
		if (ret->init())
		{
			ret->autorelease();
			return ret;
		}
		delete ret;
		return nullptr;
	}

	static CCScene *scene()
	{
		auto scene = CCScene::create();
		scene->addChild(WSRLayer::create());
		return scene;
	}

	// Store tier IDs as member variables
	std::vector<std::string> bronzeIds = {
		"59843187", "31921720", "60459825", "48812442", "57413460",
		"62619237", "34191084", "49725435", "64910644", "58527856",
		"48878216", "45756924", "67589390", "59612157", "61839007"};

	std::vector<std::string> silverIds = {
		"46570823", "66767875", "62976320", "59297291", "63103470",
		"67149625", "62243077", "59910938", "64206853", "44664220",
		"50676530", "52107163", "62990834", "63116711", "63459575",
		"62705568", "50541459", "60050144", "64144943", "65452957",
		"52648603", "64537501"};

	std::vector<std::string> goldIds = {
		"62413539", "56446182", "56279405", "65139499", "64319423",
		"64319413", "63369237", "64754276", "62308493", "64017356",
		"64048771"};

	std::vector<std::string> emeraldIds = {
		"55637201", "46650270", "63907905", "64132218", "66158978",
		"62894149", "47354575", "63329754", "64697883"};

	std::vector<std::string> rubyIds = {
		"58004788", "114160194", "44813027", "66663246", "62076892",
		"67600557", "67862379", "57696990"};

	std::vector<std::string> diamondIds = {
		"65496433", "71530517", "76663114", "65348932", "55392313",
		"64919670"};

	std::vector<std::string> amethystIds = {
		"73332806", "73199960", "50542904", "69718968", "76213334",
		"85042202"};

	std::vector<std::string> legendIds = {
		"38230726", "63006314", "79823052", "67044214"};

	bool init() override
	{
		if (!CCLayer::init())
			return false;

		auto winSize = CCDirector::get()->getWinSize();

		// Background
		auto bg = CCSprite::create("GJ_gradientBG.png");
		bg->setAnchorPoint({0, 0});
		bg->setScaleX(winSize.width / bg->getTextureRect().size.width);
		bg->setScaleY(winSize.height / bg->getTextureRect().size.height);
		bg->setColor({34, 0, 0});
		addChild(bg);

		// Calculate completions
		int bronzeCompleted = getCompletedCount(bronzeIds);
		int silverCompleted = getCompletedCount(silverIds);
		int goldCompleted = getCompletedCount(goldIds);
		int emeraldCompleted = getCompletedCount(emeraldIds);
		int rubyCompleted = getCompletedCount(rubyIds);
		int diamondCompleted = getCompletedCount(diamondIds);
		int amethystCompleted = getCompletedCount(amethystIds);
		int legendCompleted = getCompletedCount(legendIds);

		// Create tier cells
		auto scrollLayer = ScrollLayer::create({356.0f, 220.0f});
		scrollLayer->m_contentLayer->setLayout(
			ColumnLayout::create()
				->setAxisReverse(true)
				->setAutoGrowAxis(220.0f)
				->setGap(0.0f));

		struct Tier
		{
			std::string name;
			int completed;
			int total;
		};

		std::vector<Tier> tiers = {
			{"Bronze", bronzeCompleted, static_cast<int>(bronzeIds.size())},
			{"Silver", silverCompleted, static_cast<int>(silverIds.size())},
			{"Gold", goldCompleted, static_cast<int>(goldIds.size())},
			{"Emerald", emeraldCompleted, static_cast<int>(emeraldIds.size())},
			{"Ruby", rubyCompleted, static_cast<int>(rubyIds.size())},
			{"Diamond", diamondCompleted, static_cast<int>(diamondIds.size())},
			{"Amethyst", amethystCompleted, static_cast<int>(amethystIds.size())},
			{"Legend", legendCompleted, static_cast<int>(legendIds.size())}};

		float yPos = 180;
		for (size_t i = 0; i < tiers.size(); i++)
		{
			auto &tier = tiers[i];

			// Check if previous tier is unlocked
			bool isLocked = false;
			if (i > 0)
			{
				auto &prevTier = tiers[i - 1];
				int requiredCompletions = (tier.name == "Silver")	  ? 9
										  : (tier.name == "Gold")	  ? 13
										  : (tier.name == "Emerald")  ? 5
										  : (tier.name == "Ruby")	  ? 4
										  : (tier.name == "Diamond")  ? 3
										  : (tier.name == "Amethyst") ? 2
										  : (tier.name == "Legend")	  ? 1
																	  : 0;
				isLocked = prevTier.completed < requiredCompletions;

				if (isLocked)
				{
					auto lockedCell = createLockedCell(tier.name, requiredCompletions, prevTier.name);
					scrollLayer->m_contentLayer->addChild(lockedCell);
				}
			}
			if (!isLocked)
			{
				auto cell = TierCell::create(tier.name, tier.completed, tier.total,
											 this,
											 menu_selector(WSRLayer::onViewTier));
				cell->setPosition({0, yPos});
				scrollLayer->m_contentLayer->addChild(cell);
			}
			yPos -= 40;
		}

		scrollLayer->m_contentLayer->updateLayout();
		scrollLayer->moveToTop();

		m_listLayer = GJListLayer::create(nullptr, "WSR Tiers", {0, 0, 0, 180}, 356.f, 220.f, 0);
		m_listLayer->setPosition(winSize / 2 - m_listLayer->getContentSize() / 2);
		m_listLayer->addChild(scrollLayer);
		addChild(m_listLayer);

		// Back button
		auto menu = CCMenu::create();
		menu->setPosition({0, 0});
		auto backBtn = CCMenuItemSpriteExtra::create(
			CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png"),
			this, menu_selector(WSRLayer::onBack));
		backBtn->setPosition({25, winSize.height - 25});
		menu->addChild(backBtn);
		addChild(menu);

		setKeypadEnabled(true);
		return true;
	}

	int getCompletedCount(const std::vector<std::string> &tierIds)
	{
		int completed = 0;
		auto gsm = GameStatsManager::sharedState();

		for (const auto &idStr : tierIds)
		{
			int levelId = geode::utils::numFromString<int>(idStr).unwrapOr(0);
			if (levelId == 0)
				continue;

			if (gsm->hasCompletedOnlineLevel(levelId))
			{
				completed++;
			}
		}

		return completed;
	}

	void onViewTier(CCObject *sender)
	{
		auto btn = static_cast<CCMenuItemSpriteExtra *>(sender);
		auto tierName = static_cast<CCString *>(btn->getUserObject())->getCString();

		auto tierLevelIds = getTierLevelIds(tierName);

		auto scene = TierLevelsLayer::scene(tierName, tierLevelIds);
		CCDirector::get()->pushScene(CCTransitionFade::create(0.5f, scene));
	}

	std::vector<std::string> getTierLevelIds(const std::string &tierName)
	{
		if (tierName == "Bronze")
			return bronzeIds;
		else if (tierName == "Silver")
			return silverIds;
		else if (tierName == "Gold")
			return goldIds;
		else if (tierName == "Emerald")
			return emeraldIds;
		else if (tierName == "Ruby")
			return rubyIds;
		else if (tierName == "Diamond")
			return diamondIds;
		else if (tierName == "Amethyst")
			return amethystIds;
		else if (tierName == "Legend")
			return legendIds;
		return {};
	}

private:
	void onBack(CCObject *)
	{
		CCDirector::get()->popSceneWithTransition(0.5f, kPopTransitionFade);
	}

	void keyBackClicked() override
	{
		onBack(nullptr);
	}

	GJListLayer *m_listLayer = nullptr;
};

class $modify(WSRLevelSearchLayer, LevelSearchLayer)
{
	bool init(int type)
	{
		if (!LevelSearchLayer::init(type))
			return false;

		auto wsrSprite = CircleButtonSprite::createWithSprite("wave.png"_spr);
		wsrSprite->setRotation(45.0f);
		wsrSprite->setScale(0.8f);

		auto wsrButton = CCMenuItemSpriteExtra::create(
			wsrSprite,
			this,
			menu_selector(WSRLevelSearchLayer::onWSRButton));

		wsrButton->setID("wave.png"_spr);

		auto menu = this->getChildByID("other-filter-menu");
		menu->addChild(wsrButton);
		menu->updateLayout();

		return true;
	}

	void onWSRButton(CCObject *sender)
	{
		CCDirector::get()->pushScene(CCTransitionFade::create(0.5f, WSRLayer::scene()));
	}
};