#pragma once
#include "stdafx.h"
#include "Engine/Core/EngineStructs.h"
#include "Engine/Live/LiveMemory.h"
#include "Frontend/StrucGraph.h"

/****************************************
*										*
*	+++ UEDumper live editor +++		*
*	This class contains the interface	*
*	of the live editor					*
*										*
****************************************/


namespace windows
{
	class LiveEditor
	{
		// avoid stack overflows if the user expands too many nodes
		// ideally we need to write this as a loop instead of recursively to avoid this
		static const int MAX_RECURSION_DEPTH_SAFEGUARD = 100;

		static inline bool liveEditorStarted = false;

		enum EditorTabType
		{
			TabTypeAddress,
			TabTypeOffset,
			TabTypeAdvanced
		};

		struct EditorTab
		{
			EditorTabType type;
			bool found = false;
			std::string name;
			uint64_t address = 0; //use for TabTypeAddress or TabTypeOffset
			std::string origin = ""; // use for TabTypeAdvanced or TabTypeOffset
			EngineStructs::Struct* struc;
			bool isClass = false;
		};

		static inline bool guessSuperClass = true;

		static inline std::unordered_map<uint64_t, std::string> realSuperClassCache{};

		static inline bool bRenderAddInspector = false;
		static inline bool bRenderSearchResults = false;
		static inline bool autoscrollToMember = false;

		static inline int tabPicked = 0;

		static inline std::vector<EditorTab> tabs{};

		static inline char searchText[512] = { 0 };
		static inline std::string previousSearchText;
		static inline int searchResultPicked = 0;
		static inline std::string searchResultMember;
		static inline std::vector<Node> searchResults;
		static inline std::set<std::pair<uint64_t, int>> offsetsToExpand;
		static inline bool bFindingPaths; // whether or not a search is in progress

		static void populateStrucGraph(EngineStructs::Struct *struc);
		static void populateStrucGraph(EngineStructs::Struct *struc, EngineStructs::Struct* parent);

		static void renderAddAddress();

		static void renderAddOffset();

		static void renderAddInspector();
		

		static std::string appendSecret(const std::string& secret, const std::string& newName, int offset);


		/**
		 * \brief draws a field to support read/write.
		 * \param block memory block for the member
		 * \param offset the offset in the block 
		 * \param bitOffset the bit offset
		 * \param isBit if its a bit
		 * \param type fieldType
		 * \param secret a secret key to make the field unique
		 */
		static void drawReadWriteableField(LiveMemory::MemoryBlock* block, const int offset, const int bitOffset, bool isBit, const fieldType& type, const std::string& secret);


		//add your prop here for support!


		/**
		 * \brief TArray support for members
		 * \param member member that is a TArray
		 * \param block memory block the member lies in
		 * \param secret secret
		 * \param innerOffset inner offset
		 * \param the recursion depth so far
		 */
		static void drawMemberArrayProperty(const EngineStructs::Member& member, LiveMemory::MemoryBlock* block, const std::string& secret, int innerOffset, uint64_t parentAddr, int depth);

		/**
		 * \brief displays the given struct for a memory block
		 * \param struc the struct that gets displayed
		 * \param name the name of the member
		 * \param block the memory block where the data lies
		 * \param secret a secret key to make the members unique
		 * \param offset data offset so it will use block + offset for the members
		 * \param depth the recursion depth so far
		 */
		static void drawStructProperty(const EngineStructs::Struct* struc, const std::string& name, LiveMemory::MemoryBlock* block, const std::string& secret, int offset, int depth);

		/**
		 * \brief draws a single member that is not clickable and has support to get written to
		 * \param member the member
		 * \param block the block where the memory lies in
		 * \param innerOffset additional offset to the member.offset
		 * \param secret a secret key to make the member unique
		 * \param simple simple displaying or extended
		 */
		static void drawNonclickableMember(const EngineStructs::Member& member, LiveMemory::MemoryBlock* block, int innerOffset, const std::string& secret, bool simple = false);

		/**
		 * \brief displays a ObjectProperty which is a pointer
		 * \param member the member
		 * \param block the memory block where the data lies
		 * \param secret a secret key to make the member unique
		 * \param innerOffset additional offset to the member.offset
		 * \param depth the recursion depth so far
		 */
		static void drawMemberObjectProperty(const EngineStructs::Member& member, LiveMemory::MemoryBlock* block, const std::string& secret, int innerOffset, uint64_t parentAddr, int depth);

		/**
		 * \brief displays a TEnumAsByte
		 * \param member the member
		 * \param subEnum the enum of the TEnum
		 * \param block the memory block where the data lies
		 * \param secret a secret key to make the member unique
		 * \param innerOffset additional offset to the member.offset
		 */
		static void drawTEnumAsByteProperty(const EngineStructs::Member& member, const EngineStructs::Enum* subEnum, LiveMemory::MemoryBlock* block, const std::string& secret, int innerOffset);
		

		/**
		 * \brief 
		 * \param struc the struct type what gets displayed
		 * \param address the real game address (for the memory block)
		 * \param secret a secret for imgui and their string ids
		 * \param innerOffset in case a struct is in a struct (no ptr) we wanna display those fields too. We can do this by setting a inner offset
		 * which is just the real struct + offset to the inner struct (member) and keeping the ADDRESS THE SAME. With this we also have click support
		 * \param depth the recursion depth so far
		 */
		static void drawMembers(const EngineStructs::Struct* struc, uint64_t address, const std::string& secret, int innerOffset = 0, int depth = 0);

		/**
		 * \brief displays the entire struct/class properly
		 * \param struc struct to display
		 * \param address game address
		 * \param name name of the struct
		 * \param secret a secret key to make the members unique
		 * \param origin whether the game address should be displayed or a specific origin
		 */
		static void renderStruct(const EngineStructs::Struct* struc, uint64_t address, const std::string& name, const std::string& secret, const std::string& origin = "", int depth = 0);

		/**
		 * \brief checks whether the StructName is valid. Enabling lookForBest does additionally memory operations
		 * \param classPointer pointer to the class UObject
		 * \param CName name of the Struct (if lookForBest fails this will be used)
		 * \param outPackageIndex returning package index
		 * \param outStructIndex returning struct index
		 * \param isClass whether its in the struct vector or class vector
		 * \param lookForBest check whether it should look for the "real" class
		 * \return if the struct and returning data is valid
		 */
		static bool isValidStructName(uint64_t classPointer, const std::string& CName, EngineStructs::Struct*& outStruct, bool lookForBest = false);

		/**
		 * \brief checks whether the EnumName is valid.
		 * \param CName thr name of the enum
		 * \param outPackageIndex returning package index
		 * \param outEnumIndex returning enum index
		 * \return if the enum and returning data is valid
		 */
		static bool isValidEnumName(const std::string& CName, EngineStructs::Enum*& enu);

		static bool renderSearchBox();
		static void renderSearchResults();

		static void performSearch();
		static void performSearch(const std::string searchString);
		

	public:
		LiveEditor();

		static void renderLiveEditor();

		static bool LiveEditorStarted();

		static bool renderLaunchPopup();

		static bool renderQuitPopup();

		static void renderEditPopUp();

		/**
		 * \brief callback function that has to get called at the end of every frame in case
		 * there's something that has to be rendered topmost. Use carefully!
		 */
		static void topmostCallback();

		static std::string convertToLowercase(const std::string& str);
	};
}