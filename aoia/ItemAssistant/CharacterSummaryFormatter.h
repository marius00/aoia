#ifndef CHARACTERSUMMARYFORMATTER_H
#define CHARACTERSUMMARYFORMATTER_H

#include "DataModel.h"

namespace aoia { namespace sv {

    /**
     * \brief
     * This class generates a HTML table with a toon summary for the supplied summary model.
     */
    class CharacterSummaryFormatter
    {
    public:
        CharacterSummaryFormatter(DataModelPtr model, std::tstring const& summaryHeading = _T(""));

        std::tstring toString() const;

    private:
        DataModelPtr m_model;
        std::tstring m_summaryHeading;
    };

}}  // namespace

#endif // CHARACTERSUMMARYFORMATTER_H
