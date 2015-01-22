#ifndef CSVEXPORTER_H
#define CSVEXPORTER_H

#include <datagrid/IDataGridModel.h>


namespace aoia
{
    class CSVExporter
    {
    public:
        CSVExporter();
        ~CSVExporter();

        void DoExport(std::tostream &os, IDataGridModelPtr data) const;

    protected:
        std::tstring EnsureEncapsulation(std::tstring const& field) const;
    };
}

#endif // CSVEXPORTER_H
