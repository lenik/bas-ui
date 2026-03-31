#include "EditHistory.hpp"

// IEditHistory default implementations
int IEditHistory::getUndoCount() const {
    return -1;  // unknown by default
}

int IEditHistory::getRedoCount() const {
    return -1;  // unknown by default
}

// VirtualEditHistory::Record implementation
VirtualEditHistory::Record::Record(VirtualEditHistory* history, std::string description)
    : m_history(history)
    , m_description(std::move(description))
{
}

// EditList implementation
void EditList::discardHistory()
{
    preserveUndoHistory(0);
    preserveRedoHistory(0);
}

int VirtualEditHistory::getUndoCount() const
{
    return canUndo() ? -1 : 0;
}

std::shared_ptr<IEditRecord> VirtualEditHistory::getUndoRecord(int index) const
{
    int count = IEditHistory::getUndoCount();
    if (index < 0 || (count >= 0 && index >= count)) {
        throw std::out_of_range("index out of range");
    }
    return std::make_shared<Record>(const_cast<VirtualEditHistory*>(this), "Edit");
}

int VirtualEditHistory::getRedoCount() const
{
    return canRedo() ? -1 : 0;
}

std::shared_ptr<IEditRecord> VirtualEditHistory::getRedoRecord(int index) const
{
    int count = IEditHistory::getRedoCount();
    if (index < 0 || (count >= 0 && index >= count)) {
        throw std::out_of_range("index out of range");
    }
    return std::make_shared<Record>(const_cast<VirtualEditHistory*>(this), "Redo");
}


EditList::EditList()
    : m_history()
    , m_position(0)
{
}

EditList::~EditList()
{
}

int EditList::getUndoCount() const
{
    return m_position;
}

std::shared_ptr<IEditRecord> EditList::getUndoRecord(int index) const
{
    return m_history[index];
}

int EditList::getRedoCount() const
{
    return m_history.size() - m_position - 1;
}

std::shared_ptr<IEditRecord> EditList::getRedoRecord(int index) const
{
    return m_history[m_position + index];
}

void EditList::commit(std::shared_ptr<IEditRecord> record)
{
    record->commit();
    preserveRedoHistory(0);
    m_history.push_back(std::move(record));
    m_position++;
}

void EditList::undo()
{
    if (m_position <= 0)
        return;
    int old_pos = m_position;
    m_position--;
    onhistorymove(m_position, old_pos);
}

void EditList::redo()
{
    if (m_position >= m_history.size() - 1)
        return;
    int old_pos = m_position;
    m_position++;
    onhistorymove(m_position, old_pos);
}

void EditList::preserveUndoHistory(int limit)
{
    int n_erase = limit > m_position ? 0 : m_position - limit;
    if (n_erase > 0) {
        // delete from beginning
        m_history.erase(m_history.begin(), m_history.begin() + n_erase);
        m_position -= n_erase;
    }
}

void EditList::preserveRedoHistory(int limit)
{
    int n_redo = m_history.size() - m_position - 1;
    int n_erase = limit > n_redo ? 0 : n_redo - limit;
    if (n_erase > 0) {
        // delete from the last
        m_history.erase(m_history.end() - n_erase, m_history.end());
    }
}
