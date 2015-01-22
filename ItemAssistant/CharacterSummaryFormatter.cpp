#include "stdafx.h"
#include "CharacterSummaryFormatter.h"
#include <boost/format.hpp>


namespace aoia
{
    namespace sv
    {
        CharacterSummaryFormatter::CharacterSummaryFormatter(DataModelPtr model, std::tstring const& summaryHeading)
            : m_model(model)
            , m_summaryHeading(summaryHeading)
        {
        }


        std::tstring CharacterSummaryFormatter::toString() const
        {
            std::tostringstream out;
            __int64 totalCredits = 0;
            int totalLevels = 0;
            int totalAiLevels = 0;

            out.imbue(std::locale(""));

            if (!m_summaryHeading.empty())
            {
                out << "<h2>" << m_summaryHeading << "</h2>";
            }
            out << "<table id=\"summarytbl\"><tr>";

            for (unsigned int j = 0; j < m_model->getColumnCount(); ++j)
            {
                out << "<th>" << m_model->getColumnName(j) << "</th>";
            }

            out << "</tr>";

            for (unsigned int i = 0; i < m_model->getItemCount(); ++i)
            {
                out << "<tr>";
                for (unsigned int j = 0; j < m_model->getColumnCount(); ++j)
                {
                    if (j == 0)
                    {
                        out << "<td class=\"leftalign\">";
                    }
                    else
                    {
                        out << "<td class=\"rightalign\">";
                    }

                    std::tstring propertyValue = m_model->getItemProperty(i, j);
                    if (j > 0 && propertyValue.compare(_T("-")) != 0)
                    {
                        try
                        {
                            unsigned int value = boost::lexical_cast<unsigned int>(propertyValue);
							if (j != 5)
								out << value;
							else {
								if (value == 15)
									out << 4;
								else if (value == 7)
									out << 3;
								else if (value == 3)
									out << 2;
								else if (value == 1)
									out << 1;
								else 
									out << "-";
							}

                            // Update aggregated values
                            switch(j)
                            {
                            case 1:
                                totalLevels += value;
                                break;
                            case 2:
                                totalAiLevels += value;
                                break;
                            case 3:
                                totalCredits += value;
                                break;
                            }
                        }
                        catch (boost::bad_lexical_cast&)
                        {
                            out << propertyValue;
                        }
                    }
                    else
                    {
                        out << propertyValue;
                    }
                    out << "</td>";
                }
                out << "</tr>";
            }

            out << "<tr>";
            for (unsigned int j = 0; j < m_model->getColumnCount(); ++j)
            {
                if (j == 0)
                {
                    out << "<td class=\"leftalign\">SUM</td>";
                }
                else if (j == 1)
                {
                    // Level
                    out << "<td class=\"rightalign\">" << totalLevels << "</td>";
                }
                else if (j == 2)
                {
                    // AI level
                    out << "<td class=\"rightalign\">" << totalAiLevels << "</td>";
                }
                else if (j == 3)
                {
                    // Credits
                    out << "<td class=\"rightalign\">" << totalCredits << "</td>";
                }
            }
            out << "</tr>";
            out << "</table>";
            return out.str();
        }
    }

}  // namespace
