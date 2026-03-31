#ifndef EDITHISTORY_H
#define EDITHISTORY_H

#include <boost/signals2.hpp>

#include <memory>
#include <string>
#include <vector>

class IEditRecord {
public:
    virtual ~IEditRecord() {}
    
    virtual std::string description() const = 0;
    virtual void commit() = 0;
    virtual void rollback() = 0;
};

class IEditHistory {
public:
    virtual ~IEditHistory() {}

    virtual bool canUndo() const { return getUndoCount() != 0; }
    virtual bool canRedo() const { return getRedoCount() != 0; }

    // -1 for unknown
    virtual int getUndoCount() const = 0;
    virtual std::shared_ptr<IEditRecord> getUndoRecord(int index) const = 0;

    // -1 for unknown
    virtual int getRedoCount() const = 0;
    virtual std::shared_ptr<IEditRecord> getRedoRecord(int index) const = 0;

    virtual void discardHistory() {
        preserveUndoHistory(0);
        preserveRedoHistory(0);
    }
    virtual void preserveUndoHistory(int limit) {}
    virtual void preserveRedoHistory(int limit) {}

    virtual void undo() = 0;
    virtual void redo() = 0;
    
public:
    // -1 if pos is unknown
    using historymove_signal_t = boost::signals2::signal<void(int new_pos, int old_pos)>;
    using historymove_slot_t = typename historymove_signal_t::slot_type;
    
    historymove_signal_t onhistorymove;
};

class VirtualEditHistory : public IEditHistory {
public:
    class Record : public IEditRecord {
    public:
        Record(VirtualEditHistory* history, std::string description);
        virtual std::string description() const override { return m_description; }
        virtual void commit() override {
            m_history->redo();
        }
        virtual void rollback() override {
            m_history->undo();
        }
    private:
        VirtualEditHistory* m_history;
        std::string m_description;
    };

    virtual int getUndoCount() const override;
    virtual std::shared_ptr<IEditRecord> getUndoRecord(int index) const override;
    virtual int getRedoCount() const override;
    virtual std::shared_ptr<IEditRecord> getRedoRecord(int index) const override;

    virtual bool canUndo() const = 0;
    virtual bool canRedo() const = 0;
};

class EditList : public IEditHistory {
public:
    EditList();
    virtual ~EditList();

    virtual int getUndoCount() const override;
    virtual std::shared_ptr<IEditRecord> getUndoRecord(int index) const override;
    virtual int getRedoCount() const override;
    virtual std::shared_ptr<IEditRecord> getRedoRecord(int index) const override;

    virtual void commit(std::shared_ptr<IEditRecord> record);

    virtual void undo() override;
    virtual void redo() override;
    
    virtual void preserveUndoHistory(int limit) override;
    virtual void preserveRedoHistory(int limit) override;
    virtual void discardHistory();

private:
    std::vector<std::shared_ptr<IEditRecord>> m_history;
    int m_position;
    // int m_maxHistorySize;
};

#endif // EDITHISTORY_H