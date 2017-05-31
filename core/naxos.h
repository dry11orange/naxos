/// @file
/// Unnecessary definitions for the XCSP3 Mini-Solver competition
///
/// Part of https://github.com/pothitos/naxos

#ifndef Ns_NAXOS_H
#define Ns_NAXOS_H

#include "naxos-mini.h"

namespace naxos {

class Ns_ConstrXinDomain : public Ns_Constraint {

    private:
        NsIntVar* VarX;

        NsInt min;
        NsDeque<NsInt>* domainPrevious;
        NsDeque<NsInt>* domainNext;
        bool arraysAllocated;

    public:
        Ns_ConstrXinDomain(NsIntVar* X, const NsDeque<NsInt>& domain,
                           NsDeque<NsInt>* domainPrevious_init,
                           NsDeque<NsInt>* domainNext_init);

        virtual ~Ns_ConstrXinDomain(void)
        {
                if (arraysAllocated) {
                        delete domainPrevious;
                        delete domainNext;
                }
        }

        virtual int varsInvolvedIn(void) const
        {
                return 1;
        }

        virtual void toGraphFile(std::ofstream& fileConstraintsGraph) const
        {
                fileConstraintsGraph << "\n\t//Var" << VarX << " -> Dom"
                                     << &domainPrevious
                                     << " [label=\"dom\"];\n";
        }

        virtual void ArcCons(void);
        virtual void LocalArcCons(Ns_QueueItem& Qitem);
};

class Ns_ConstrCount : public Ns_Constraint {

    private:
        NsIntVarArray* VarArr;

        typedef Ns_UNORDERED_MAP<Ns_pointer_t, NsIndex> VarIndex_t;

        VarIndex_t VarIndex;

    public:
        typedef Ns_UNORDERED_MAP<NsInt, NsIndex> ValueIndex_t;

    private:
        ValueIndex_t ValueIndex;

    public:
        struct ValueOccurrence_t {

                NsInt value;

                NsInt occurrence;

                const NsDeque<NsIndex>* split_positions;

                NsIntVarArray vCount;

                ValueOccurrence_t(const NsInt value_init) : value(value_init)
                {
                }

                ValueOccurrence_t(const NsInt value_init,
                                  const NsInt occurrence_init,
                                  NsProblemManager& pm)
                  : value(value_init),
                    occurrence(occurrence_init),
                    split_positions(0)
                {
                        assert_Ns(occurrence >= 0,
                                  "Ns_ConstrCount::ValueOccurrence_t::"
                                  "ValueOccurrence_t: negative value in "
                                  "'occurrence'");
                        vCount.push_back(NsIntVar(pm, 0, occurrence));
                }

                ValueOccurrence_t(const NsInt value_init,
                                  const NsInt occurrence_init,
                                  NsProblemManager& pm,
                                  const NsDeque<NsIndex>& split_positions_init,
                                  const NsIndex Split)
                  : value(value_init),
                    occurrence(occurrence_init),
                    split_positions(&split_positions_init)
                {
                        assert_Ns(occurrence >= 0,
                                  "Ns_ConstrCount::ValueOccurrence_t::"
                                  "ValueOccurrence_t: negative value in "
                                  "'occurrence'");
                        for (NsIndex i = 0; i < split_positions->size(); ++i)
                                vCount.push_back(NsIntVar(pm, 0, Split));
                        vCount.push_back(NsIntVar(pm, 0, occurrence % Split));
                        if (split_positions->size() == 0)
                                split_positions = 0;
                }

                NsIndex splitIndexForPosition(NsIndex i)
                {
                        if (split_positions == 0)
                                return 0;
                        NsDeque<NsIndex>::const_iterator cit =
                            lower_bound(split_positions->begin(),
                                        split_positions->end(), i);
                        return (cit - split_positions->begin());
                }

                bool operator<(const ValueOccurrence_t& Y) const
                {
                        return (value < Y.value);
                }
        };

    private:
        NsDeque<ValueOccurrence_t> ValuesOccurrences;
        const NsIndex Split;
        const NsIndex Dwin;

    public:
        Ns_ConstrCount(NsIntVarArray* VarArr_init, const NsDeque<NsInt>& Values,
                       const NsDeque<NsInt>& Occurrences,
                       const NsDeque<NsDeque<NsIndex>>& SplitPositions,
                       const NsIndex Split_init, const NsIndex Dwin_init);

        virtual int varsInvolvedIn(void) const
        {
                return VarArr->size();
        }

        virtual void toGraphFile(std::ofstream& fileConstraintsGraph) const
        {
                Ns_arrayConstraintToGraphFile(fileConstraintsGraph, VarArr,
                                              this, "count");
        }

        virtual void ArcCons(void);
        virtual void LocalArcCons(Ns_QueueItem& Qitem);
};

class Ns_ExprInDomain : public Ns_Expression {

    private:
        NsProblemManager& pm;
        NsDeque<NsInt>& domain;
        NsDeque<NsInt>* domainPrevious;
        NsDeque<NsInt>* domainNext;

    public:
        Ns_ExprInDomain(NsProblemManager& pm_init, NsDeque<NsInt>& domain_init,
                        NsDeque<NsInt>* domainPrevious_init,
                        NsDeque<NsInt>* domainNext_init)
          : pm(pm_init),
            domain(domain_init),
            domainPrevious(domainPrevious_init),
            domainNext(domainNext_init)
        {
        }

        virtual void post(NsIntVar& VarX) const;
        virtual NsIntVar& post(void) const;
};

inline Ns_ExprInDomain NsInDomain(NsProblemManager& pm, NsDeque<NsInt>& domain)
{
        return Ns_ExprInDomain(pm, domain, 0, 0);
}

inline Ns_ExprInDomain NsInDomain(NsProblemManager& pm, NsDeque<NsInt>& domain,
                                  NsDeque<NsInt>& domainPrevious,
                                  NsDeque<NsInt>& domainNext)
{
        return Ns_ExprInDomain(pm, domain, &domainPrevious, &domainNext);
}

class Ns_ExprConstrCount : public Ns_ExprConstr {

    private:
        NsIntVarArray& VarArr;
        const NsDeque<NsInt>& Values;
        const NsDeque<NsInt>& Occurrences;
        const NsDeque<NsDeque<NsIndex>>& SplitPositions;
        const NsIndex Split;
        const NsIndex Dwin;

    public:
        Ns_ExprConstrCount(NsIntVarArray& Arr,
                           const NsDeque<NsInt>& Values_init,
                           const NsDeque<NsInt>& Occurrences_init,
                           const NsDeque<NsDeque<NsIndex>>& SplitPositions_init,
                           const NsIndex Split_init, const NsIndex Dwin_init)
          : Ns_ExprConstr(true),
            VarArr(Arr),
            Values(Values_init),
            Occurrences(Occurrences_init),
            SplitPositions(SplitPositions_init),
            Split(Split_init),
            Dwin(Dwin_init)
        {
        }

        virtual Ns_Constraint* postConstraint(bool positively) const;

        virtual void postC(NsIntVar& /*VarX*/, bool /*positively*/) const
        {
                throw NsException("Ns_ExprConstrCount::postC: NsCount cannot "
                                  "be used as a meta-constraint");
        }
        virtual NsIntVar& postC(bool /*positively*/) const
        {
                throw NsException("Ns_ExprConstrCount::postC: NsCount cannot "
                                  "be used as a meta-constraint");
        }
};

inline Ns_ExprConstrCount
NsCount(NsIntVarArray& Arr, const NsDeque<NsInt>& Values,
        const NsDeque<NsInt>& Occurrences,
        const NsDeque<NsDeque<NsIndex>>& SplitPositions =
            NsDeque<NsDeque<NsIndex>>(),
        const NsIndex Split = 0, const NsIndex Dwin = 1)
{
        return Ns_ExprConstrCount(Arr, Values, Occurrences, SplitPositions,
                                  Split, Dwin);
}

} // end namespace

#endif // Ns_NAXOS_H
